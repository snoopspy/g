#include "hostanalyzer.h"

HostAnalyzer::HostAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m2\"", "su -c \"nexutil -k1\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	hostMgr_.pcapDevice_ = &pcapDevice_;
	hostWatch_.pcapDevice_ = &pcapDevice_;
	hostWatch_.hostMgr_ = &hostMgr_;

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

	return true;
}

bool HostAnalyzer::doClose() {
	pcapDevice_.close();
	hostMgr_.close();
	hostWatch_.close();
	return true;
}
