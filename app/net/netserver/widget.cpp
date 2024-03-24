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

	QObject::connect(&tcpServer_, &QTcpServer::newConnection, this, &Widget::doNewConnection);
	QObject::connect(&sslServer_, &QSslServer::newConnection, this, &Widget::doNewConnection);
}

void Widget::finiControl() {
	on_pbClose_clicked();
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
	if (netServer_ == &tcpServer_)
		active = tcpServer_.isListening();
	else if (netServer_ == &udpSocket_)
		active = udpSocket_.state() != QAbstractSocket::UnconnectedState;
	else if (netServer_ == &sslServer_)
		active = sslServer_.isListening();

	ui->pbOpen->setEnabled(!active);
	ui->pbClose->setEnabled(active);
	ui->pbSend->setEnabled(active);
}

void Widget::addText(QString msg) {
	ui->pteRecv->moveCursor(QTextCursor::End);
	ui->pteRecv->insertPlainText(msg);
	ui->pteRecv->moveCursor(QTextCursor::End);
}

void Widget::showError(QString error) {
	QString msg = "[error] " + error + "\r\n";
	addText(msg);
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
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	QByteArray ba = socket->readAll();
	if (ui->chkShowHexa->isChecked())
		ba = ba.toHex();
	ba += "\r\n";
	addText(ba);
}

void Widget::doConnected() {
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	Q_ASSERT(socket != nullptr);
	QString msg = "[connected] " + socket->peerAddress().toString() + "\r\n";
	addText(msg);
}

void Widget::doDisconnected() {
	QAbstractSocket* socket = dynamic_cast<QAbstractSocket*>(sender());
	Q_ASSERT(socket != nullptr);
	QString msg = "[disconnected] " + socket->peerAddress().toString() + "\r\n";
	addText(msg);
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
	if (socketError == QAbstractSocket::UnknownSocketError) return;
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QMetaEnum menum = mobj.enumerator(mobj.indexOfEnumerator("SocketError"));
	QString key = menum.valueToKey(socketError);
	QString msg = "[error] " + key + "\r\n";
	addText(msg);
	setControl();
}

void Widget::doNewConnection() {
	//tcpServer_.hasPendingConnections()
}

void Widget::doPendingConnectionAvailable() {

}

void Widget::showOption(NetServer* netServer) {
	GProp::showDialog(netServer);
}

void Widget::on_pbOpen_clicked() {
	int currentIndex = ui->tabOption->currentIndex();
	switch (currentIndex) {
		case TcpTab:
			netServer_ = &tcpServer_;
			if (!tcpServer_.listen(QHostAddress(option_.tcpServer_.localHost_), ui->leTcpPort->text().toUShort()))
				showError(tcpServer_.errorString());
			break;
		case UdpTab:
			netServer_ = &udpSocket_;
			if (!udpSocket_.bind(QHostAddress(option_.udpServer_.localHost_), ui->leUdpPort->text().toUShort()))
				showError(udpSocket_.errorString());
			break;
		case SslTab:
			netServer_ = &sslServer_;
			if (!sslServer_.listen(QHostAddress(option_.sslServer_.localHost_), ui->leSslPort->text().toUShort()))
				showError(sslServer_.errorString());
			break;
	}
	setControl();
}

void Widget::on_pbClose_clicked() {
	if (netServer_ == &tcpServer_)
		tcpServer_.close();
	else if (netServer_ == &udpSocket_)
		udpSocket_.close();
	else if (netServer_ == &sslServer_)
		sslServer_.close();
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
	if (netServer_ == nullptr) return;
	QByteArray ba = qPrintable(ui->pteSend->toPlainText());
	ba = ba.replace("\n", "\r\n");
	if (ui->chkSendHexa->isChecked()) ba = ba.fromHex(ba);
	//if (netServer_ == &tcpServer_)
	// netClient_->write(ba); // gilgil temp 2024.03.24
}
