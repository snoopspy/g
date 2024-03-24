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

	QObject::connect(&tcpSocket_, &QTcpSocket::connected, this, &Widget::doConnected);
	QObject::connect(&tcpSocket_, &QTcpSocket::disconnected, this, &Widget::doDisconnected);
	QObject::connect(&tcpSocket_, &QTcpSocket::errorOccurred, this, &Widget::doErrorOccurred);
	QObject::connect(&tcpSocket_, &QTcpSocket::stateChanged, this, &Widget::doStateChanged);
	QObject::connect(&tcpSocket_, &QTcpSocket::readyRead, this, &Widget::doReadyRead);

	QObject::connect(&udpSocket_, &QUdpSocket::readyRead, this, &Widget::doReadyRead);

	QObject::connect(&sslSocket_, &QTcpSocket::connected, this, &Widget::doConnected);
	QObject::connect(&sslSocket_, &QTcpSocket::disconnected, this, &Widget::doDisconnected);
	QObject::connect(&sslSocket_, &QSslSocket::errorOccurred, this, &Widget::doErrorOccurred);
	QObject::connect(&sslSocket_, &QSslSocket::stateChanged, this, &Widget::doStateChanged);
	QObject::connect(&sslSocket_, &QTcpSocket::readyRead, this, &Widget::doReadyRead);
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
	bool active = false;
	if (netSocket_) {
		switch (netSocket_->state()) {
			case QAbstractSocket::UnconnectedState: active = false; break;
			case QAbstractSocket::HostLookupState: active = true; break;
			case QAbstractSocket::ConnectingState: active = true; break;
			case QAbstractSocket::ConnectedState: active = true; break;
			case QAbstractSocket::BoundState: active = true; break;
			case QAbstractSocket::ListeningState: active = true; break;
			case QAbstractSocket::ClosingState: active = true; break;
		}
	}
	ui->pbOpen->setEnabled(!active);
	ui->pbClose->setEnabled(active);
	ui->pbSend->setEnabled(netSocket_ == nullptr ? false : netSocket_->state() == QAbstractSocket::ConnectedState);
}

void Widget::addText(QString msg) {
	ui->pteRecv->moveCursor(QTextCursor::End);
	ui->pteRecv->insertPlainText(msg);
	ui->pteRecv->moveCursor(QTextCursor::End);
}

void Widget::doConnected() {
	QString msg = "[connected] " + netSocket_->peerAddress().toString() + "\r\n";
	addText(msg);
}

void Widget::doDisconnected() {
	QString msg = "[disconnected] " + netSocket_->peerAddress().toString() + "\r\n";
	addText(msg);
}

void Widget::doErrorOccurred(QAbstractSocket::SocketError socketError) {
	Q_UNUSED(socketError)
	QString msg = "[error] " + netSocket_->errorString() + "\r\n";
	addText(msg);
	setControl();
}

void Widget::doStateChanged(QAbstractSocket::SocketState socketState) {
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QMetaEnum menum = mobj.enumerator(mobj.indexOfEnumerator("SocketState"));
	QString key = menum.valueToKey(socketState);
	qDebug() << "[stateChanged]" << QString::number(socketState) << key;
	setControl();
}

void Widget::doReadyRead() {
	QByteArray ba = netSocket_->readAll();
	if (ui->chkShowHexa->isChecked())
		ba = ba.toHex();
	ba += "\r\n";
	addText(ba);
}

void Widget::showOption(NetClient* netClient) {
	GProp::showDialog(netClient);
}

void Widget::on_pbOpen_clicked() {
	int currentIndex = ui->tabOption->currentIndex();
	switch (currentIndex) {
		case 0:
			netSocket_ = &tcpSocket_;
			tcpSocket_.bind(QHostAddress(option_.tcpClient_.localHost_), option_.tcpClient_.localPort_, QAbstractSocket::DefaultForPlatform | QAbstractSocket::ReuseAddressHint);
			tcpSocket_.connectToHost(ui->leTcpHost->text(), ui->leTcpPort->text().toUShort());
			break;
		case 1:
			netSocket_ = &udpSocket_;
			udpSocket_.bind(QHostAddress(option_.udpClient_.localHost_), option_.udpClient_.localPort_, QAbstractSocket::DefaultForPlatform | QAbstractSocket::ReuseAddressHint);
			udpSocket_.connectToHost(ui->leUdpHost->text(), ui->leUdpPort->text().toUShort());
			break;
		case 2:
			netSocket_ = &sslSocket_;
			sslSocket_.setProtocol(QSsl::SslProtocol(option_.sslClient_.protocol_));
			sslSocket_.bind(QHostAddress(option_.sslClient_.localHost_), option_.sslClient_.localPort_, QAbstractSocket::DefaultForPlatform | QAbstractSocket::ReuseAddressHint);
			sslSocket_.connectToHostEncrypted(ui->leSslHost->text(), ui->leSslPort->text().toUShort());
			break;
	}
	setControl();
}

void Widget::on_pbClose_clicked() {
	if (netSocket_ != nullptr) {
		netSocket_->disconnectFromHost();
		netSocket_->abort();
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
	if (netSocket_ == nullptr) return;
	QByteArray ba = ui->pteSend->toPlainText().toUtf8();
	ba = ba.replace("\n", "\r\n");
	if (ui->chkSendHexa->isChecked()) ba = ba.fromHex(ba);
	netSocket_->write(ba);
}
