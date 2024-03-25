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

void Widget::prepareTcpServer(QTcpServer* server) {
	QObject::connect(server, &QTcpServer::acceptError, this, &Widget::doAcceptError);
	QObject::connect(server, &QTcpServer::newConnection, this, &Widget::doNewConnection);
	QObject::connect(server, &QTcpServer::pendingConnectionAvailable, this, &Widget::doPendingConnectionAvailable);
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
	qDebug() << "";

	QByteArray ba;

	QUdpSocket* udpSocket = dynamic_cast<QUdpSocket*>(sender());
	if (udpSocket != nullptr) {
		QNetworkDatagram datagram = udpSocket->receiveDatagram();
		ba = datagram.data();
		if (ui->chkEcho->isChecked()) {
			// UDP broadcast not supported yet
			datagram.setDestination(datagram.senderAddress(), datagram.senderPort());
			udpSocket->writeDatagram(datagram);
		}
	} else {
		QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
		Q_ASSERT(socket != nullptr);
		ba = socket->readAll();
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
	if (socketError == QAbstractSocket::UnknownSocketError) return;
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QString value = mobj.enumerator(mobj.indexOfEnumerator("SocketError")).valueToKey(socketError);
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

void Widget::doAcceptError(QAbstractSocket::SocketError socketError) {
	qDebug() << "";
	if (socketError == QAbstractSocket::UnknownSocketError) return;
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QMetaEnum menum = mobj.enumerator(mobj.indexOfEnumerator("SocketError"));
	QString key = menum.valueToKey(socketError);
	QString msg = "[accept error] " + key + "\r\n";
	addText(msg, true);
	setControl();
}

void Widget::doNewConnection() {
	qDebug() << "";
	QTcpServer* server = dynamic_cast<QTcpServer*>(sender());
	Q_ASSERT(server != nullptr);

	while (server->hasPendingConnections()) {
		qDebug() << "has!!!";
		QAbstractSocket* socket = server->nextPendingConnection();
		QString msg = QString("[connected] %1:%2\r\n").arg(socket->peerAddress().toString()).arg(QString::number(socket->peerPort()));
		addText(msg, true);
		prepareAbstractSocket(socket);

		socketList_.push_back(socket);
	}
}

void Widget::doPendingConnectionAvailable() {
	qDebug() << "";
	doNewConnection();
}

void Widget::showOption(NetServer* netServer) {
	GProp::showDialog(netServer);
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
		case SslTab:
			QSslServer* sslServer = new QSslServer(this);

			QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());

			sslConfiguration.setProtocol(QSsl::SslProtocol(option_.sslServer_.protocol_));

			QFile keyFile(option_.sslServer_.keyFileName_);
			if (!keyFile.open(QIODevice::ReadOnly)) {
				showError(QString("can not open file %1").arg(option_.sslServer_.keyFileName_));
				break;
			}
			QSslKey sslKey(&keyFile, QSsl::Rsa);
			sslConfiguration.setPrivateKey(sslKey);
			keyFile.close();

			QFile crtFile(option_.sslServer_.crtFileName_);
			if (!crtFile.open(QIODevice::ReadOnly)) {
				showError(QString("can not open file %1").arg(option_.sslServer_.crtFileName_));
				break;
			}
			QSslCertificate sslCertificate(&crtFile);
			sslConfiguration.setLocalCertificate(sslCertificate);
			crtFile.close();

			sslServer->setSslConfiguration(sslConfiguration);

			prepareTcpServer(sslServer);
			currServer_ = sslServer;
			if (!sslServer->listen(QHostAddress(option_.sslServer_.localHost_), ui->leSslPort->text().toUShort()))
				showError(sslServer->errorString());
			break;
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
