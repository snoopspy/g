#include "hostdialog.h"
#include <QGridLayout>

#include <GJson>

HostDialog::HostDialog(QWidget* parent, GHostDb* hostDb, GMac mac) : QDialog(parent), hostDb_(hostDb), mac_(mac) {
	resize(QSize(640, 480));
	setWindowTitle("Host");

	GHostDb::Item item;
	if (!hostDb_->selectHost(mac, &item)) {
		qWarning() << QString("hostDb_->selectHost(%1) return false").arg(QString(mac));
		return;
	}

	QGridLayout* layout = new QGridLayout(this);

	QLabel* lblMac = new QLabel("Mac", this); leMac_ = new QLineEdit(QString(mac), this);
	QLabel* lblIp = new QLabel("IP", this); leIp_ = new QLineEdit(QString(item.ip_), this);
	QLabel* lblAlias  = new QLabel("Alias", this); leAlias_ = new QLineEdit(item.alias_, this);
	QLabel* lblHost = new QLabel("Host", this); leHost_ = new QLineEdit(item.host_, this);
	QLabel* lblVendor = new QLabel("Vendor", this); leVendor_ = new QLineEdit(item.vendor_, this);
	QLabel* lblMode = new QLabel("Mode", this); cbMode_ = new QComboBox(this); cbMode_->addItems(QStringList{"Default", "Allow", "Block"});

	leMac_->setReadOnly(true);
	leIp_->setReadOnly(true);
	leHost_->setReadOnly(true);
	leVendor_->setReadOnly(true);

	layout->addWidget(lblMac, 0, 0); layout->addWidget(leMac_, 0, 1);
	layout->addWidget(lblIp, 1, 0); layout->addWidget(leIp_, 1, 1);
	layout->addWidget(lblAlias, 2, 0); layout->addWidget(leAlias_, 2, 1);
	layout->addWidget(lblHost, 3, 0); layout->addWidget(leHost_, 3, 1);
	layout->addWidget(lblVendor, 4, 0); layout->addWidget(leVendor_, 4, 1);
	layout->addWidget(lblMode, 5, 0); layout->addWidget(cbMode_, 5, 1);

	setLayout(layout);
}

HostDialog::~HostDialog() {

}

void HostDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
}

void HostDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
}
