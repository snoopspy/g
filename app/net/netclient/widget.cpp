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

	QObject::connect(&tcpSocket_, &QTcpSocket::connected, this, &Widget::connected);
	QObject::connect(&tcpSocket_, &QTcpSocket::disconnected, this, &Widget::disconnected);
	QObject::connect(&tcpSocket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
	QObject::connect(&tcpSocket_, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
	QObject::connect(&tcpSocket_, &QTcpSocket::readyRead, this, &Widget::readyRead);
	QObject::connect(&udpSocket_, &QUdpSocket::readyRead, this, &Widget::readyRead);
	QObject::connect(&sslSocket_, &QTcpSocket::connected, this, &Widget::connected);
	QObject::connect(&sslSocket_, &QTcpSocket::disconnected, this, &Widget::disconnected);
	QObject::connect(&sslSocket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
	QObject::connect(&sslSocket_, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
	QObject::connect(&sslSocket_, &QTcpSocket::readyRead, this, &Widget::readyRead);
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
	ui->pteSend->insertPlainText(jo["sendText"].toString());
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
	if (netClient_) {
		switch (netClient_->state()) {
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
	ui->pbSend->setEnabled(netClient_ == nullptr ? false : netClient_->state() == QAbstractSocket::ConnectedState);
}

void Widget::connected() {
	QString msg = "[connected] " + netClient_->peerAddress().toString() + "\r\n";
	ui->pteRecv->insertPlainText(msg);
}

void Widget::disconnected() {
	QString msg = "[disconnected] " + netClient_->peerAddress().toString() + "\r\n";
	ui->pteRecv->insertPlainText(msg);
}

void Widget::error(QAbstractSocket::SocketError socketError) {
	Q_UNUSED(socketError)
	QString msg = "[error] " + netClient_->errorString() + "\r\n";
	ui->pteRecv->insertPlainText(msg);
	setControl();
}

void Widget::stateChanged(QAbstractSocket::SocketState socketState) {
	const QMetaObject& mobj = QAbstractSocket::staticMetaObject;
	QMetaEnum menum = mobj.enumerator(mobj.indexOfEnumerator("SocketState"));
	QString key = menum.valueToKey(socketState);
	qDebug() << "[stateChanged]" << QString::number(socketState) << key;
	setControl();
}

void Widget::readyRead() {
	QByteArray ba = netClient_->readAll();
	if (ui->chkShowHexa->isChecked())
		ba = ba.toHex();
	ba += "\r\n";
	ui->pteRecv->insertPlainText(ba);
}

void Widget::showOption(NetClient* netClient) {
	GProp::showDialog(netClient);
}

void Widget::on_pbOpen_clicked() {
	int currentIndex = ui->tabOption->currentIndex();
	switch (currentIndex) {
		case 0:
			netClient_ = &tcpSocket_;
			tcpSocket_.connectToHost(ui->leTcpHost->text(), ui->leTcpPort->text().toUShort());
			break;
		case 1:
			netClient_ = &udpSocket_;
			udpSocket_.connectToHost(ui->leUdpHost->text(), ui->leUdpPort->text().toUShort());
			break;
		case 2:
			netClient_ = &sslSocket_;
			sslSocket_.connectToHostEncrypted(ui->leSslHost->text(), ui->leSslPort->text().toUShort());
			break;
	}
	setControl();
}

void Widget::on_pbClose_clicked() {
	if (netClient_ != nullptr) {
		netClient_->disconnectFromHost();
		netClient_->abort();
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
	if (netClient_ == nullptr) return;
	QByteArray ba = qPrintable(ui->pteSend->toPlainText());
	ba = ba.replace("\n", "\r\n");
	if (ui->chkSendHexa->isChecked()) ba = ba.fromHex(ba);
	netClient_->write(ba);
}
