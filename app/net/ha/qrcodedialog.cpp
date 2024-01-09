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
	if (!tcpServer_->listen(QHostAddress("127.0.0.1"), port_)) {
		QMessageBox mb;
		mb.warning(this, "Error", QString("Can not open port %1").arg(port_));
	}

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->addWidget(lblQrCode_);
	vLayout->addWidget(pbGenerate_);
	this->setLayout(vLayout);

	QObject::connect(pbGenerate_, &QPushButton::clicked, this, &QrCodeDialog::pbGenerate_clicked);
}

QrCodeDialog::~QrCodeDialog() {
	tcpServer_->close();
}

void QrCodeDialog::pbGenerate_clicked(bool checked) {
	(void)checked;

	QString text = "10.1.1.3/12345678"; // gilgil temp 2024.01.08
	int size = std::min(lblQrCode_->width(), lblQrCode_->height());
	QImage image = generator_->generateQr(text, size);
	lblQrCode_->setPixmap(QPixmap::fromImage(image));
}

void QrCodeDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
}

void QrCodeDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
}
