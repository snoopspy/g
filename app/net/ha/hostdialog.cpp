#include "hostdialog.h"

#include <GJson>

HostDialog::HostDialog(QWidget* parent, GMac mac, HostAnalyzer* hostAnalyzer, GHostMgr::HostValue* hostValue)
	: QDialog(parent), mac_(mac), ha_(hostAnalyzer), hv_(hostValue) {
	resize(QSize(640, 480));
	setWindowTitle("Host");

	GHostDb::Item* dbItem = ha_->hostDb_.getItem(hv_);

	QGridLayout* gLayout = new QGridLayout();
	{
		QLabel* lblMac = new QLabel("Mac", this); leMac_ = new QLineEdit(QString(mac), this);
		QLabel* lblIp = new QLabel("IP", this); leIp_ = new QLineEdit(QString(dbItem->ip_), this);
		QLabel* lblAlias  = new QLabel("Alias", this); leAlias_ = new QLineEdit(dbItem->alias_, this);
		QLabel* lblHost = new QLabel("Host", this); leHost_ = new QLineEdit(dbItem->host_, this);
		QLabel* lblVendor = new QLabel("Vendor", this); leVendor_ = new QLineEdit(dbItem->vendor_, this);
		QLabel* lblMode = new QLabel("Mode", this); cbMode_ = new QComboBox(this); cbMode_->addItems(QStringList{"Default", "Allow", "Block"});
		cbMode_->setCurrentIndex(int(dbItem->mode_));
		QLabel* lblBlockTime = new QLabel("BlockTime", this); dteBlockTime_ = new QDateTimeEdit(this);
		dteBlockTime_->setDisplayFormat("MM/dd hh:mm");
		dteBlockTime_->setDateTime(QDateTime::fromSecsSinceEpoch(ha_->getItem(hv_)->blockTime_));

		leMac_->setEnabled(false);
		leIp_->setEnabled(false);
		leHost_->setEnabled(false);
		leVendor_->setEnabled(false);

		gLayout->addWidget(lblMac, 0, 0); gLayout->addWidget(leMac_, 0, 1);
		gLayout->addWidget(lblIp, 1, 0); gLayout->addWidget(leIp_, 1, 1);
		gLayout->addWidget(lblAlias, 2, 0); gLayout->addWidget(leAlias_, 2, 1);
		gLayout->addWidget(lblHost, 3, 0); gLayout->addWidget(leHost_, 3, 1);
		gLayout->addWidget(lblVendor, 4, 0); gLayout->addWidget(leVendor_, 4, 1);
		gLayout->addWidget(lblMode, 5, 0); gLayout->addWidget(cbMode_, 5, 1);
		gLayout->addWidget(lblBlockTime, 6, 0); gLayout->addWidget(dteBlockTime_, 6, 1);
	}

	QHBoxLayout* hLayout = new QHBoxLayout();
	{
		pbOk_ = new QPushButton("Ok", this); hLayout->insertWidget(0, pbOk_);
		pbCancel_ = new QPushButton("Cancel", this); hLayout->insertWidget(1, pbCancel_);
	}

	QVBoxLayout* vLayout = new QVBoxLayout();
	{
		vLayout->insertLayout(0, gLayout, 1);
		vLayout->insertLayout(1, hLayout);
	}

	setLayout(vLayout);

	QObject::connect(leAlias_, &QLineEdit::textChanged, this, &HostDialog::setControl);
	QObject::connect(cbMode_, SIGNAL(currentIndexChanged(int)), this, SLOT(setControl2(int)));
	QObject::connect(dteBlockTime_, &QDateTimeEdit::dateTimeChanged, this, &HostDialog::setControl);
	QObject::connect(pbOk_, &QPushButton::clicked, this, &HostDialog::pbOk_clicked);
	QObject::connect(pbCancel_, &QPushButton::clicked, this, &HostDialog::pbCancel_clicked);

	pbOk_->setEnabled(false);
}

HostDialog::~HostDialog() {
}

void HostDialog::setControl() {
	pbOk_->setEnabled(true);
}

void HostDialog::setControl2(int index) {
	(void)index;
	pbOk_->setEnabled(true);
}

void HostDialog::pbOk_clicked() {
	GHostDb::Item* dbItem = ha_->hostDb_.getItem(hv_);

	dbItem->alias_ = leAlias_->text();
	dbItem->mode_ = GHostDb::Mode(cbMode_->currentIndex());
	bool res = ha_->hostDb_.updateHost(mac_, dbItem);
	if (!res) {
		qWarning() << QString("hostDb_.updateHost(%1) return false").arg(QString(mac_));
		return;
	}

	HostAnalyzer::Item* haItem = ha_->getItem(hv_);
	haItem->state_ = HostAnalyzer::Item::Changed;
	haItem->blockTime_ = dteBlockTime_->dateTime().toSecsSinceEpoch();
	ha_->checkBlockTime(hv_);

	accept();
}

void HostDialog::pbCancel_clicked() {
	reject();
}

void HostDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
}

void HostDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
}

