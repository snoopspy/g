#include "hostdialog.h"

#include <GJson>

HostDialog::HostDialog(QWidget* parent, GMac mac, HostAnalyzer* hostAnalyzer)
	: QDialog(parent), mac_(mac), ha_(hostAnalyzer) {
	resize(QSize(640, 480));
	setWindowTitle("Host");

	GHostDb::Item dbItem;
	if (!ha_->hostDb_.selectHost(mac, &dbItem)) {
		qWarning() << QString("selectHost(%1) return false").arg(QString(mac));
	}

	QGridLayout* gLayout = new QGridLayout();
	{
		QLabel* lblMac = new QLabel("Mac", this); leMac_ = new QLineEdit(QString(mac), this);
		QLabel* lblIp = new QLabel("IP", this); leIp_ = new QLineEdit(QString(dbItem.ip_), this);
		QLabel* lblAlias  = new QLabel("Alias", this); leAlias_ = new QLineEdit(dbItem.alias_, this);
		QLabel* lblHost = new QLabel("Host", this); leHost_ = new QLineEdit(dbItem.host_, this);
		QLabel* lblVendor = new QLabel("Vendor", this); leVendor_ = new QLineEdit(dbItem.vendor_, this);
		QLabel* lblMode = new QLabel("Mode", this); cbMode_ = new QComboBox(this); cbMode_->addItems(QStringList{"Auto", "Allow", "Block"});
		cbMode_->setCurrentIndex(int(dbItem.mode_));
		QLabel* lblBlockTime = new QLabel("BlockTime", this); dteBlockTime_ = new QDateTimeEdit(this);

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

	QObject::connect(leAlias_, &QLineEdit::textChanged, this, &HostDialog::leAlias_TextChanged);
	QObject::connect(cbMode_, SIGNAL(currentIndexChanged(int)), this, SLOT(cdMode_currentIndexChanged(int)));
	QObject::connect(pbOk_, &QPushButton::clicked, this, &HostDialog::pbOk_clicked);
	QObject::connect(pbCancel_, &QPushButton::clicked, this, &HostDialog::pbCancel_clicked);

	setDateTimeEdit();
}

HostDialog::~HostDialog() {
}

void HostDialog::setDateTimeEdit() {
	GHostDb::Mode mode = GHostDb::Mode(cbMode_->currentIndex());
	if (!ha_->active() || ha_->admitTimeoutSec_ == 0 || mode != GHostDb::Auto) {
		dteBlockTime_->setEnabled(false);
		dteBlockTime_->setDateTime(QDateTime::fromSecsSinceEpoch(0));
		dteBlockTime_->setDisplayFormat("m");
	} else {
		dteBlockTime_->setEnabled(true);
		GHostMgr::HostMap* hostMap = &ha_->hostMgr_.hostMap_;
		QMutexLocker ml(hostMap);
		GHostMgr::HostMap::iterator it = hostMap->find(mac_);
		Q_ASSERT(it != hostMap->end());
		GHostMgr::HostValue* hostValue = it.value();
		Q_ASSERT(hostValue != nullptr);
		HostAnalyzer::Item* haItem = ha_->getItem(hostValue);
		Q_ASSERT(haItem != nullptr);
		time_t blockTime = haItem->blockTime_;
		if (blockTime == 0)
			blockTime = hostValue->firstTime_ + ha_->admitTimeoutSec_;
		dteBlockTime_->setDateTime(QDateTime::fromSecsSinceEpoch(blockTime));
		dteBlockTime_->setDisplayFormat("MM/dd hh:mm");
	}
}

void HostDialog::leAlias_TextChanged() {
}

void HostDialog::cdMode_currentIndexChanged(int index) {
	(void)index;
	setDateTimeEdit();
}

void HostDialog::pbOk_clicked() {
	GHostDb::Item dbItem;
	if (!ha_->hostDb_.selectHost(mac_, &dbItem)) {
		qWarning() << QString("selectHost(%1) return false").arg(QString(mac_));
	}

	dbItem.alias_ = leAlias_->text();
	dbItem.mode_ = GHostDb::Mode(cbMode_->currentIndex());
	if (!ha_->hostDb_.updateHost(mac_, &dbItem)) {
		qWarning() << QString("hostDb_.updateHost(%1) return false").arg(QString(mac_));
	}

	GHostMgr::HostMap* hostMap = &ha_->hostMgr_.hostMap_;
	QMutexLocker ml(hostMap);
	GHostMgr::HostMap::iterator it = hostMap->find(mac_);
	if (it != hostMap->end()) {
		GHostMgr::HostValue* hostValue = it.value();
		Q_ASSERT(hostValue != nullptr);
		HostAnalyzer::Item* haItem = ha_->getItem(hostValue);
		haItem->state_ = HostAnalyzer::Item::Changed;
		if (dbItem.mode_ == GHostDb::Auto)
			haItem->blockTime_ = dteBlockTime_->dateTime().toSecsSinceEpoch();
		else
			haItem->blockTime_ = 0;
		ha_->checkBlockTime(hostValue);
	}

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

