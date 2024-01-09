#include "qrcodedialog.h"

#include <GJson>

QrCodeDialog::QrCodeDialog(QWidget* parent) : QDialog(parent) {
	resize(QSize(640, 480));
	setWindowTitle("QR Code");

	lblQrCode_ = new QLabel(this);
	pbGenerate_ = new QPushButton(this);
	tcpServer_ = new QTcpServer(this);
	generator_ = new QrCodeGenerator(this);

	lblQrCode_->setAlignment(Qt::AlignCenter);
	pbGenerate_->setText("Generate QR Code");

	QObject::connect(tcpServer_, &QTcpServer::newConnection, this, &QrCodeDialog::tcpServer_newConnection, Qt::DirectConnection);
	if (!tcpServer_->listen(QHostAddress::Any, port_)) {
		QMessageBox mb;
		mb.warning(this, "Error", QString("Can not open port %1").arg(port_));
	}

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->addWidget(lblQrCode_);
	vLayout->addWidget(pbGenerate_);
	this->setLayout(vLayout);

	QObject::connect(pbGenerate_, &QPushButton::clicked, this, &QrCodeDialog::pbGenerate_clicked);

	std::srand(QDateTime::currentSecsSinceEpoch());
}

QrCodeDialog::~QrCodeDialog() {
	tcpServer_->close();
}

QrCodeDialog::Session::Session() {
	created_ = QDateTime::currentSecsSinceEpoch();
	for (int i = 0; i < Session::SessionSize; i++)
		bytes_[i] = std::rand() % 10 + '0';
}

QString QrCodeDialog::Session::bytesToString() {
	QString res;
	for (int i = 0; i < Session::SessionSize; i++)
		res += bytes_[i];
	return res;
}

void QrCodeDialog::SessionList::deleteOldSession() {
	time_t now = QDateTime::currentSecsSinceEpoch();
	int i = this->count();
	for (i = 0; i < count();) {
		const Session& session = at(i);
		time_t created = session.created_;
		if (created + qrCodeDialog_->sessionTimeOutSec_ < now) {
			this->removeAt(i);
		} else
			i++;
	}
}

#include "hawidget.h"
void QrCodeDialog::pbGenerate_clicked(bool checked) {
	(void)checked;

	sessionList_.deleteOldSession();

	Session session;
	qDebug() << session.bytesToString(); // gilgil temp 2024.01.10
	sessionList_.push_back(session);

	HaWidget* haWidget = dynamic_cast<HaWidget*>(parent());
	Q_ASSERT(haWidget != nullptr);
	GIntf* intf = haWidget->hostAnalyzer_.pcapDevice_.intf();
	Q_ASSERT(intf != nullptr);
	GIp myIp = intf->ip();

	QString text = QString("http://%1:%2/%3").arg(QString(myIp)).arg(port_).arg(session.bytesToString());
	int size = std::min(lblQrCode_->width(), lblQrCode_->height());
	QImage image = generator_->generateQr(text, size);
	lblQrCode_->setPixmap(QPixmap::fromImage(image));
}

void QrCodeDialog::tcpServer_newConnection() {
	QTcpSocket *socket = tcpServer_->nextPendingConnection();
	socket->waitForReadyRead();
	QByteArray ba = socket->readAll();
	qDebug() << ba;

	socket->write("HTTP/1.1 200 Ok\r\n\r\nOk");
	socket->close();
}

void QrCodeDialog::propLoad(QJsonObject jo) {
	port_ = jo["port"].toInt(Port);
	sessionTimeOutSec_ = jo["sessionTimeOutSec"].toInt(Session::SessionSize);
	jo["rect"] >> GJson::rect(this);
}

void QrCodeDialog::propSave(QJsonObject& jo) {
	jo["port"] = port_;
	jo["sessionTimeOutSec"] = sessionTimeOutSec_;
	jo["rect"] << GJson::rect(this);
}
