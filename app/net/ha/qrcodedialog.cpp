#include "qrcodedialog.h"

#include <GJson>

QrCodeDialog::QrCodeDialog(QWidget* parent) : QDialog(parent) {
	resize(QSize(640, 480));
	setWindowTitle("QR Code");

	lblUrl_ = new QLabel(this);
	lblQrCode_ = new QLabel(this);
	pbGenerate_ = new QPushButton(this);
	generator_ = new QrCodeGenerator(this);

	lblUrl_->setAlignment(Qt::AlignCenter);
	QSizePolicy policy = lblUrl_->sizePolicy();
	policy.setVerticalPolicy(QSizePolicy::Maximum);
	lblUrl_->setSizePolicy(policy);
	lblQrCode_->setAlignment(Qt::AlignCenter);
	pbGenerate_->setText("Generate QR Code");

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->addWidget(lblUrl_);
	vLayout->addWidget(lblQrCode_);
	vLayout->addWidget(pbGenerate_);
	this->setLayout(vLayout);

	QObject::connect(pbGenerate_, &QPushButton::clicked, this, &QrCodeDialog::pbGenerate_clicked);

	static bool srandCalled = false;
	if (!srandCalled) {
		std::srand(QDateTime::currentSecsSinceEpoch());
		srandCalled = true;
	}
}

QrCodeDialog::~QrCodeDialog() {
}

void QrCodeDialog::resizeEvent(QResizeEvent* event) {
	(void)event;

	if (firstResizeEvent_) {
		pbGenerate_->click();
		firstResizeEvent_ = false;
	}
}

#include "hawidget.h"
void QrCodeDialog::pbGenerate_clicked(bool checked) {
	(void)checked;

	HaWidget* haWidget = dynamic_cast<HaWidget*>(parent());
	Q_ASSERT(haWidget != nullptr);
	GIntf* intf = haWidget->hostAnalyzer_.pcapDevice_.intf();
	Q_ASSERT(intf != nullptr);
	GIp myIp = intf->ip();

	WebServer* webServer = &haWidget->hostAnalyzer_.webServer_;
	WebServer::Session session;
	qDebug() << session.bytesToString(); // gilgil temp 2024.01.10
	webServer->sessionList_.push_back(session);

	QString url = QString("http://%1:%2/%3").arg(QString(myIp)).arg(webServer->port_).arg(session.bytesToString());
	lblUrl_->setText(url);
	int size = std::min(lblQrCode_->width(), lblQrCode_->height());
	QImage image = generator_->generateQr(url, size * 2 / 3);
	lblQrCode_->setPixmap(QPixmap::fromImage(image));
}

void QrCodeDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
}

void QrCodeDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
}
