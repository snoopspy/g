#include "beaconflood.h"

BeaconFlood::BeaconFlood(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -c1\"", "su -c \"nexutil -d\"", "su -c \"nexutil -m2\"", "su -c \"nexutil -k1\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\"", "su -c \"ifconfig wlan0 down\""}));
#endif
}

BeaconFlood::~BeaconFlood() {
	close();
}

bool BeaconFlood::doOpen() {
	beaconFlood_.messages_ = plainTextEdit_->toPlainText().split("\n");
	if (!beaconFlood_.open()) {
		err = beaconFlood_.err;
		return false;
	}
	return true;
}

bool BeaconFlood::doClose() {
	beaconFlood_.close();
	return true;
}
