#include "hostanalyzer.h"

Q_INVOKABLE HostAnalyzer::HostAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	hostMgr_.pcapDevice_ = &pcapDevice_;
	hostWatch_.pcapDevice_ = &pcapDevice_;
	hostWatch_.hostMgr_ = &hostMgr_;
	hostScan_.pcapDevice_ = &pcapDevice_;

	// for probeDetected signal
	qRegisterMetaType<GMac>("GMac");
	QObject::connect(&pcapDevice_, &GPcapDevice::captured, &hostMgr_, &GHostMgr::manage, Qt::DirectConnection);
}

HostAnalyzer::~HostAnalyzer() {
	close();
}

bool HostAnalyzer::doOpen() {
	if (!pcapDevice_.open()) {
		err = pcapDevice_.err;
		return false;
	}

	if (!hostMgr_.open()) {
		err = hostMgr_.err;
		return false;
	}

	if (!hostWatch_.open()) {
		err = hostWatch_.err;
		return false;
	}

	if (!hostScan_.open()) {
		err = hostScan_.err;
		return false;
	}

	hostOffset_ = hostMgr_.requestItems_.request("GHostWatch-host", sizeof(QTreeWidgetItem));
	hostMgr_.managables_.insert(this);

	return true;
}

bool HostAnalyzer::doClose() {
	pcapDevice_.close();
	hostMgr_.close();
	hostWatch_.close();
	hostScan_.close();
	return true;
}

void HostAnalyzer::hostDetected(GMac mac, GHostMgr::Value* value) {
	QMetaObject::invokeMethod(this, [=]() {
		QTreeWidgetItem* item = reinterpret_cast<QTreeWidgetItem*>(value->mem(hostOffset_));
		new (item) QTreeWidgetItem(QStringList{QString(value->ip_), QString(mac)});
		treeWidget_->addTopLevelItem(item);
	});
}

void HostAnalyzer::hostDeleted(GMac mac, GHostMgr::Value* value) {
	(void)mac;
	QMetaObject::invokeMethod(this, [=]() {
		QTreeWidgetItem* item = reinterpret_cast<QTreeWidgetItem*>(value->mem(hostOffset_));
		item->~QTreeWidgetItem();
	});
}
