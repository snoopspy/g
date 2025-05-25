#include "widget.h"
#include "ui_widget.h"

#include <QDialog>
#include <GJson>
#include <GPropWidget>

// ----------------------------------------------------------------------------
// Widget
// ----------------------------------------------------------------------------
Widget::Widget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Widget) {
	ui->setupUi(this);
	initControl();
	loadControl();
	setControl();
}

Widget::~Widget() {
	saveControl();
	finiControl();
	delete ui;
}

void Widget::initControl() {
	move(0, 0); resize(640, 480);

	ui->mainLayout->setSpacing(0);
	ui->pteRecv->setWordWrapMode(QTextOption::NoWrap);
	ui->pteSend->setWordWrapMode(QTextOption::NoWrap);
}

void Widget::finiControl() {
	ui->pbClose->click();
}

void Widget::loadControl() {
	QJsonObject& jo = GJson::instance();

	jo["widget"] >> GJson::rect(this);
	jo["splitter"] >> GJson::splitterSizes(ui->splitter);
	jo["option"] >> option_;

	ui->chkShowHexa->setChecked(jo["showHexa"].toBool());
	ui->chkSendHexa->setChecked(jo["sendHexa"].toBool());
	ui->chkEcho->setChecked(jo["echo"].toBool());
	ui->chkEchoBroadcast->setChecked(jo["echoBroadcast"].toBool());
	ui->tabOption->setCurrentIndex(jo["currentIndex"].toInt());
	ui->leTcpPort->setText(jo["tcpPort"].toString());
	ui->leUdpPort->setText(jo["udpPort"].toString());
	ui->leSslPort->setText(jo["sslPort"].toString());
	ui->pteSend->setPlainText(jo["sendText"].toString());
}

void Widget::saveControl() {
	QJsonObject& jo = GJson::instance();

	jo["widget"] << GJson::rect(this);
	jo["splitter"] << GJson::splitterSizes(ui->splitter);
	jo["option"] << option_;

	jo["showHexa"] = ui->chkShowHexa->isChecked();
	jo["sendHexa"] = ui->chkSendHexa->isChecked();
	jo["echo"] = ui->chkEcho->isChecked();
	jo["echoBroadcast"] = ui->chkEchoBroadcast->isChecked();
	jo["sendHexa"] = ui->chkEchoBroadcast->isChecked();
	jo["currentIndex"]= ui->tabOption->currentIndex();
	jo["tcpPort"] = ui->leTcpPort->text();
	jo["udpPort"] = ui->leUdpPort->text();
	jo["sslPort"] = ui->leSslPort->text();
	jo["sendText"] = ui->pteSend->toPlainText();
}

void Widget::setControl() {
	bool active = false;

	QTcpServer* tcpServer = dynamic_cast<QTcpServer*>(currServer_);
	if (tcpServer != nullptr)
		active = tcpServer->isListening();

	QUdpSocket* udpSocket = dynamic_cast<QUdpSocket*>(currServer_);
	if (udpSocket != nullptr)
		active = udpSocket->state() != QAbstractSocket::UnconnectedState;

	ui->pbOpen->setEnabled(!active);
	ui->pbClose->setEnabled(active);
	ui->pbSend->setEnabled(active);
}

void Widget::addText(QString msg, bool newLine) {
	ui->pteRecv->moveCursor(QTextCursor::End);
	if (newLine && !ui->pteRecv->toPlainText().isEmpty()) {
		QString last = ui->pteRecv->toPlainText().last(1);
		if (last != "\n")
			msg = "\n" + msg;
	}
	ui->pteRecv->insertPlainText(msg);
	ui->pteRecv->moveCursor(QTextCursor::End);
}

void Widget::showError(QString error) {
	QString msg = "[error] " + error + "\r\n";
	addText(msg, true);
}

void Widget::prepareAbstractSocket(QAbstractSocket* socket) {
	// QIODevice
	QObject::connect(socket, &QIODevice::aboutToClose, this, &Widget::doAboutToClose);
	QObject::connect(socket, &QIODevice::bytesWritten, this, &Widget::doBytesWritten);
	QObject::connect(socket, &QIODevice::channelBytesWritten, this, &Widget::doChannelBytesWritten);
	QObject::connect(socket, &QIODevice::channelReadyRead, this, &Widget::doChannelReadyRead);
	QObject::connect(socket, &QIODevice::readChannelFinished, this, &Widget::doReadChannelFinished);
	QObject::connect(socket, &QIODevice::readyRead, this, &Widget::doReadyRead);

	// QAbstractSocket
	QObject::connect(socket, &QAbstractSocket::connected, this, &Widget::doConnected);
	QObject::connect(socket, &QAbstractSocket::disconnected, this, &Widget::doDisconnected);
	QObject::connect(socket, &QAbstractSocket::errorOccurred, this, &Widget::doErrorOccurred);
	QObject::connect(socket, &QAbstractSocket::hostFound, this, &Widget::doHostFound);
	QObject::connect(socket, &QAbstractSocket::proxyAuthenticationRequired, this, &Widget::doProxyAuthenticationRequired);
	QObject::connect(socket, &QAbstractSocket::stateChanged, this, &Widget::doStateChanged);
}

void Widget::prepareSslSocket(QSslSocket* socket) {
	// QSslSocket
	QObject::connect(socket, &QSslSocket::alertReceived, this, &Widget::doAlertReceived);
	QObject::connect(socket, &QSslSocket::alertSent, this, &Widget::doAlertSent);
	QObject::connect(socket, &QSslSocket::encrypted, this, &Widget::doEncrypted);
	QObject::connect(socket, &QSslSocket::encryptedBytesWritten, this, &Widget::doEncryptedBytesWritten);
	QObject::connect(socket, &QSslSocket::handshakeInterruptedOnError, this, &Widget::doHandshakeInterruptedOnError);
	QObject::connect(socket, &QSslSocket::modeChanged, this, &Widget::doModeChanged);
	QObject::connect(socket, &QSslSocket::newSessionTicketReceived, this, &Widget::doNewSessionTicketReceived);
	QObject::connect(socket, &QSslSocket::peerVerifyError, this, &Widget::doPeerVerifyError);
	QObject::connect(socket, &QSslSocket::preSharedKeyAuthenticationRequired, this, &Widget::doPreSharedKeyAuthenticationRequired);
	QObject::connect(socket, &QSslSocket::sslErrors, this, &Widget::doSslErrors);
}

void Widget::prepareTcpServer(QTcpServer* server) {
	QObject::connect(server, &QTcpServer::acceptError, this, &Widget::serverAcceptError);
	QObject::connect(server, &QTcpServer::newConnection, this, &Widget::serverNewConnection);
	QObject::connect(server, &QTcpServer::pendingConnectionAvailable, this, &Widget::serverPendingConnectionAvailable);
}

void Widget::prepareSslServer(QSslServer* server) {
	QObject::connect(server, &QSslServer::alertReceived, this, &Widget::serverAlertReceived);
	QObject::connect(server, &QSslServer::alertSent, this, &Widget::serverAlertSent);
	QObject::connect(server, &QSslServer::errorOccurred, this, &Widget::serverErrorOccurred);
	QObject::connect(server, &QSslServer::handshakeInterruptedOnError, this, &Widget::serverHandshakeInterruptedOnError);
	QObject::connect(server, &QSslServer::peerVerifyError, this, &Widget::serverPeerVerifyError);
	QObject::connect(server, &QSslServer::preSharedKeyAuthenticationRequired, this, &Widget::serverPreSharedKeyAuthenticationRequired);
	QObject::connect(server, &QSslServer::sslErrors, this, &Widget::serverSslErrors);
	QObject::connect(server, &QSslServer::startedEncryptionHandshake, this, &Widget::serverStartedEncryptionHandshake);
}

void Widget::doAboutToClose() {
	qDebug() << "";
}

void Widget::doBytesWritten(qint64 bytes) {
	qDebug() << bytes;
}

void Widget::doChannelBytesWritten(int channel, qint64 bytes) {
	qDebug() << channel << bytes;
}

void Widget::doChannelReadyRead(int channel) {
	qDebug() << channel;
}

void Widget::doReadChannelFinished() {
	qDebug() << "";
}

void Widget::doReadyRead() {
	QByteArray ba;

	QUdpSocket* udpSocket = dynamic_cast<QUdpSocket*>(sender());
	if (udpSocket != nullptr) {
		QNetworkDatagram datagram = udpSocket->receiveDatagram();
		ba = datagram.data();
		qDebug() << ba.size();
		if (ui->chkEcho->isChecked()) {
			// UDP broadcast not supported yet
			datagram.setDestination(datagram.senderAddress(), datagram.senderPort());
			udpSocket->writeDatagram(datagram);
		}
	} else {
		QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
		Q_ASSERT(socket != nullptr);
		ba = socket->readAll();
		qDebug() << ba.size();
		if (ui->chkEcho->isChecked()) {
			if (ui->chkEchoBroadcast->isChecked()) {
				for (QAbstractSocket* socket: socketList_)
					socket->write(ba);
			} else
				socket->write(ba);
		}
	}

	if (ui->chkShowHexa->isChecked())
		ba = ba.toHex();
	addText(ba, false);
}

void Widget::doConnected() {
	qDebug() << "";
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	Q_ASSERT(socket != nullptr);

	QString msg = QString("[connected] %1:%2\r\n").arg(socket->peerAddress().toString()).arg(QString::number(socket->peerPort()));
	addText(msg, true);
}

void Widget::doDisconnected() {
	qDebug() << "";
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	Q_ASSERT(socket != nullptr);

	QString msg = QString("[disconnected] %1:%2\r\n").arg(socket->peerAddress().toString()).arg(QString::number(socket->peerPort()));
	addText(msg, true);

	socketList_.removeOne(socket);
}

void Widget::doErrorOccurred(QAbstractSocket::SocketError socketError) {
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QString value = mobj.enumerator(mobj.indexOfEnumerator("SocketError")).valueToKey(socketError);
	qDebug() << value;
	showError(value);
	setControl();
}

void Widget::doHostFound() {
	qDebug() << "";
}

void Widget::doProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator) {
	qDebug() << proxy << authenticator->user(); // gilgil temp 2024.03.24
}

void Widget::doStateChanged(QAbstractSocket::SocketState socketState) {
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QString value = mobj.enumerator(mobj.indexOfEnumerator("SocketState")).valueToKey(socketState);
	qDebug() << QString::number(socketState) << value;
	setControl();
}

void Widget::doAlertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString& description) {
	(void)level;
	(void)type;
	qDebug() << description;
}

void Widget::doAlertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString& description) {
	(void)level;
	(void)type;
	qDebug() << description;
}

void Widget::doEncrypted() {
	qDebug() << "";
}

void Widget::doEncryptedBytesWritten(qint64 written) {
	qDebug() << written;
}

void Widget::doHandshakeInterruptedOnError(const QSslError& error) {
	qDebug() << error.errorString();
}

void Widget::doModeChanged(QSslSocket::SslMode mode) {
	switch (mode) {
		case QSslSocket::UnencryptedMode: qDebug() << "UnencryptedMode"; break;
		case QSslSocket::SslClientMode: qDebug() << "SslClientMode"; break;
		case QSslSocket::SslServerMode: qDebug() << "SslServerMode"; break;
	}
}

void Widget::doNewSessionTicketReceived()  {
	qDebug() << "";
}

void Widget::doPeerVerifyError(const QSslError& error) {
	qDebug() << error.errorString();
}

void Widget::doPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator* authenticator) {
	qDebug() << authenticator->identity();
}

void Widget::doSslErrors(const QList<QSslError>& errors) {
	for (const QSslError& error: errors)
		qDebug() << error.errorString();
}

void Widget::serverAcceptError(QAbstractSocket::SocketError socketError) {
	qDebug() << "";
	if (socketError == QAbstractSocket::UnknownSocketError) return;
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QMetaEnum menum = mobj.enumerator(mobj.indexOfEnumerator("SocketError"));
	QString key = menum.valueToKey(socketError);
	QString msg = "[accept error] " + key + "\r\n";
	addText(msg, true);
	setControl();
}

void Widget::serverNewConnection() {
	qDebug() << "";
}

void Widget::serverPendingConnectionAvailable() {
	qDebug() << "";
	QTcpServer* server = dynamic_cast<QTcpServer*>(sender());
	Q_ASSERT(server != nullptr);

	while (server->hasPendingConnections()) {
		QTcpSocket* socket = server->nextPendingConnection();
		QString msg = QString("[connected] %1:%2\r\n").arg(socket->peerAddress().toString()).arg(QString::number(socket->peerPort()));
		addText(msg, true);
		prepareAbstractSocket(socket);
		QSslSocket* sslSocket = dynamic_cast<QSslSocket*>(socket);
		if (sslSocket != nullptr)
			prepareSslSocket(sslSocket);

		socketList_.push_back(socket);
	}
}

void Widget::serverAlertReceived(QSslSocket *socket, QSsl::AlertLevel level, QSsl::AlertType type, const QString &description) {
	(void)socket;
	(void)level;
	(void)type;
	qDebug() << description;
}

void Widget::serverAlertSent(QSslSocket *socket, QSsl::AlertLevel level, QSsl::AlertType type, const QString &description) {
	(void)socket;
	(void)level;
	(void)type;
	qDebug() << description;
}

void Widget::serverErrorOccurred(QSslSocket *socket, QAbstractSocket::SocketError socketError) {
	(void)socket;
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QString value = mobj.enumerator(mobj.indexOfEnumerator("SocketError")).valueToKey(socketError);
	qDebug() << value;
	showError(value);
	setControl();
}

void Widget::serverHandshakeInterruptedOnError(QSslSocket *socket, const QSslError &error) {
	(void)socket;
	qDebug() << error.errorString();
}

void Widget::serverPeerVerifyError(QSslSocket *socket, const QSslError &error) {
	(void)socket;
	qDebug() << error.errorString();
}

void Widget::serverPreSharedKeyAuthenticationRequired(QSslSocket *socket, QSslPreSharedKeyAuthenticator *authenticator) {
	(void)socket;
	qDebug() << authenticator->identity();
}

void Widget::serverSslErrors(QSslSocket *socket, const QList<QSslError> &errors) {
	(void)socket;
	for (const QSslError& error: errors)
		qDebug() << error.errorString();
}

void Widget::serverStartedEncryptionHandshake(QSslSocket *socket) {
	(void)socket;
	qDebug() << "";
}

void Widget::showOption(NetServer* netServer) {
	GProp::showDialog(netServer, "optionDialog");
}

#include <QSslKey>
#include <QSslCertificate>
#include <QFile>
void Widget::on_pbOpen_clicked() {
	int currentIndex = ui->tabOption->currentIndex();
	switch (currentIndex) {
		case TcpTab: {
			QTcpServer* tcpServer = new QTcpServer(this);
			prepareTcpServer(tcpServer);
			currServer_ = tcpServer;
			if (!tcpServer->listen(QHostAddress(option_.tcpServer_.localHost_), ui->leTcpPort->text().toUShort()))
				showError(tcpServer->errorString());
			break;
		}
		case UdpTab: {
			QUdpSocket* udpSocket = new QUdpSocket(this);
			prepareAbstractSocket(udpSocket);
			currServer_ = udpSocket;
			if (!udpSocket->bind(QHostAddress(option_.udpServer_.localHost_), ui->leUdpPort->text().toUShort()))
				showError(udpSocket->errorString());
			break;
		}
		case SslTab: {
			QSslServer* sslServer = new QSslServer(this);

			QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());

			sslConfiguration.setProtocol(QSsl::SslProtocol(option_.sslServer_.protocol_));

			QFile keyFile(option_.sslServer_.keyFileName_);
			if (!keyFile.open(QIODevice::ReadOnly)) {
				showError(QString("can not open key file %1").arg(option_.sslServer_.keyFileName_));
				break;
			}
			QSslKey sslKey(&keyFile, QSsl::Rsa);
			sslConfiguration.setPrivateKey(sslKey);
			keyFile.close();

			QFile crtFile(option_.sslServer_.crtFileName_);
			if (!crtFile.open(QIODevice::ReadOnly)) {
				showError(QString("can not open crt file %1").arg(option_.sslServer_.crtFileName_));
				break;
			}
			QSslCertificate sslCertificate(&crtFile);
			sslConfiguration.setLocalCertificate(sslCertificate);
			crtFile.close();

			sslServer->setSslConfiguration(sslConfiguration);

			prepareTcpServer(sslServer);
			prepareSslServer(sslServer);
			currServer_ = sslServer;
			if (!sslServer->listen(QHostAddress(option_.sslServer_.localHost_), ui->leSslPort->text().toUShort()))
				showError(sslServer->errorString());
			break;
		}
	}
	setControl();
}

void Widget::on_pbClose_clicked() {
	QTcpServer* tcpServer = dynamic_cast<QTcpServer*>(currServer_);
	if (tcpServer != nullptr) { // QTcpServer or QSslServer
		tcpServer->close();
		delete tcpServer;
		currServer_ = nullptr;
	}

	QUdpSocket* udpSocket = dynamic_cast<QUdpSocket*>(currServer_);
	if (udpSocket != nullptr) {
		udpSocket->close();
		delete udpSocket;
		currServer_ = nullptr;
	}

	setControl();
}

void Widget::on_pbClear_clicked() {
	ui->pteRecv->clear();
}

void Widget::on_tbTcpAdvance_clicked() {
	option_.tcpServer_.port_ = ui->leTcpPort->text().toInt();
	showOption(&option_.tcpServer_);
	ui->leTcpPort->setText(QString::number(option_.tcpServer_.port_));
}

void Widget::on_tbUdpAdvance_clicked() {
	option_.udpServer_.port_ = ui->leUdpPort->text().toInt();
	showOption(&option_.udpServer_);
	ui->leUdpPort->setText(QString::number(option_.udpServer_.port_));
}

void Widget::on_tbSslAdvanced_clicked() {
	option_.sslServer_.port_ = ui->leSslPort->text().toInt();
	showOption(&option_.sslServer_);
	ui->leSslPort->setText(QString::number(option_.sslServer_.port_));
}

void Widget::on_pbSend_clicked() {
	if (currServer_ == nullptr) return;
	QByteArray ba = qPrintable(ui->pteSend->toPlainText());
	ba = ba.replace("\n", "\r\n");
	if (ui->chkSendHexa->isChecked()) ba = ba.fromHex(ba);
	for(QAbstractSocket* socket: socketList_)
		socket->write(ba);
}
