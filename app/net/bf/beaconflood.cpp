#include "beaconflood.h"

BeaconFlood::BeaconFlood(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -c1\"", "su -c \"nexutil -d\"", "su -c \"nexutil -m2\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\"", "su -c \"ifconfig wlan0 down\""}));
#endif
}

BeaconFlood::~BeaconFlood() {
	close();
}

bool BeaconFlood::doOpen() {
	GThreadMgr::suspendStart();

	bool isOk = false;
	while (true) {
		beaconFlood_.messages_ = plainTextEdit_->toPlainText().split("\n");
		if (!beaconFlood_.open()) {
			err = beaconFlood_.err;
			break;
		}

		channelHop_.intfName_ = beaconFlood_.intfName_;
		channelHop_.channelList_ = QStringList(QString::number(beaconFlood_.channel_));
		if (!channelHop_.open()) {
			err = channelHop_.err;
			break;
		}
		isOk = true;
		break;
	}

	GThreadMgr::resumeStart();
	return isOk;
}

bool BeaconFlood::doClose() {
	beaconFlood_.close();
	channelHop_.close();
	return true;
}
