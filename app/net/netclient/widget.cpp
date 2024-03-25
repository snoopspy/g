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
	ui->tabOption->setCurrentIndex(jo["currentIndex"].toInt());
	ui->leTcpHost->setText(jo["tcpHost"].toString());
	ui->leTcpPort->setText(jo["tcpPort"].toString());
	ui->leUdpHost->setText(jo["udpHost"].toString());
	ui->leUdpPort->setText(jo["udpPort"].toString());
	ui->leSslHost->setText(jo["sslHost"].toString());
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
	jo["currentIndex"]= ui->tabOption->currentIndex();
	jo["tcpHost"] = ui->leTcpHost->text();
	jo["tcpPort"] = ui->leTcpPort->text();
	jo["udpHost"] = ui->leUdpHost->text();
	jo["udpPort"] = ui->leUdpPort->text();
	jo["sslHost"] = ui->leSslHost->text();
	jo["sslPort"] = ui->leSslPort->text();
	jo["sendText"] = ui->pteSend->toPlainText();
}

void Widget::setControl() {
	if (currSocket_ == nullptr) {
		ui->pbOpen->setEnabled(true);
		ui->pbClose->setEnabled(false);
		ui->pbSend->setEnabled(false);
	} else {
		QAbstractSocket::SocketState state = currSocket_->state();
		ui->pbOpen->setEnabled(state == QAbstractSocket::UnconnectedState);
		ui->pbClose->setEnabled(state != QAbstractSocket::UnconnectedState);
		ui->pbSend->setEnabled(state == QAbstractSocket::ConnectedState);
	}
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
	} else {
		QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
		Q_ASSERT(socket != nullptr);
		ba = socket->readAll();
		qDebug() << ba.size();
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

void Widget::showOption(NetClient* netClient) {
	GProp::showDialog(netClient);
}

void Widget::on_pbOpen_clicked() {
	int currentIndex = ui->tabOption->currentIndex();
	switch (currentIndex) {
		case TcpTab: {
			QTcpSocket* tcpSocket = new QTcpSocket(this);
			prepareAbstractSocket(tcpSocket);
			currSocket_ = tcpSocket;
			if (!tcpSocket->bind(QHostAddress(option_.tcpClient_.localHost_), option_.tcpClient_.localPort_, QAbstractSocket::DefaultForPlatform | QAbstractSocket::ReuseAddressHint)) {
				showError(tcpSocket->errorString());
				break;
			}
			tcpSocket->connectToHost(ui->leTcpHost->text(), ui->leTcpPort->text().toUShort());
			break;
		}
		case UdpTab: {
			QUdpSocket* udpSocket = new QUdpSocket(this);
			prepareAbstractSocket(udpSocket);
			currSocket_ = udpSocket;
			if (!udpSocket->bind(QHostAddress(option_.udpClient_.localHost_), option_.udpClient_.localPort_, QAbstractSocket::DefaultForPlatform | QAbstractSocket::ReuseAddressHint)) {
				showError(udpSocket->errorString());
				break;
			}
			udpSocket->connectToHost(ui->leUdpHost->text(), ui->leUdpPort->text().toUShort());
			break;
		}
		case SslTab: {
			QSslSocket* sslSocket = new QSslSocket(this);
			sslSocket->setProtocol(QSsl::SslProtocol(option_.sslClient_.protocol_));
			prepareAbstractSocket(sslSocket);
			currSocket_ = sslSocket;
			if (!sslSocket->bind(QHostAddress(option_.sslClient_.localHost_), option_.sslClient_.localPort_, QAbstractSocket::DefaultForPlatform | QAbstractSocket::ReuseAddressHint))  {
				showError(sslSocket->errorString());
				break;
			}
			sslSocket->connectToHostEncrypted(ui->leSslHost->text(), ui->leSslPort->text().toUShort());
			break;
		}
	}
	setControl();
}

void Widget::on_pbClose_clicked() {
	if (currSocket_ != nullptr) {
		currSocket_->abort();
		currSocket_->close();
		delete currSocket_;
		currSocket_ = nullptr;
	}
	setControl();
}

void Widget::on_pbClear_clicked() {
	ui->pteRecv->clear();
}

void Widget::on_tbTcpAdvance_clicked() {
	option_.tcpClient_.host_ = ui->leTcpHost->text();
	option_.tcpClient_.port_ = ui->leTcpPort->text().toUShort();
	showOption(&option_.tcpClient_);
	ui->leTcpHost->setText(option_.tcpClient_.host_);
	ui->leTcpPort->setText(QString::number(option_.tcpClient_.port_));
}

void Widget::on_tbUdpAdvance_clicked() {
	option_.udpClient_.host_ = ui->leUdpHost->text();
	option_.udpClient_.port_ = ui->leUdpPort->text().toUShort();
	showOption(&option_.udpClient_);
	ui->leUdpHost->setText(option_.udpClient_.host_);
	ui->leUdpPort->setText(QString::number(option_.udpClient_.port_));
}

void Widget::on_tbSslAdvanced_clicked() {
	option_.sslClient_.host_ = ui->leSslHost->text();
	option_.sslClient_.port_ = ui->leSslPort->text().toUShort();
	showOption(&option_.sslClient_);
	ui->leSslHost->setText(option_.sslClient_.host_);
	ui->leSslPort->setText(QString::number(option_.sslClient_.port_));
}

void Widget::on_pbSend_clicked() {
	if (currSocket_ == nullptr) return;
	QByteArray ba = ui->pteSend->toPlainText().toUtf8();
	ba = ba.replace("\n", "\r\n");
	if (ui->chkSendHexa->isChecked()) ba = ba.fromHex(ba);
	currSocket_->write(ba);
}
