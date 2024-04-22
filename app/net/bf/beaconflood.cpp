#include "beaconflood.h"

BeaconFlood::BeaconFlood(QObject* parent) : GGraph(parent) {
	beaconFlood_.setObjectName("beaconFlood_");
	channelHop_.setObjectName("channelHop_");
	command_.setObjectName("command_");

#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -c1\"", "su -c \"nexutil -d\"", "su -c \"nexutil -m2\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\"", "su -c \"ifconfig wlan0 down\""}));
#endif

	nodes_.append(&beaconFlood_);
	nodes_.append(&channelHop_);
}

BeaconFlood::~BeaconFlood() {
	close();
}

bool BeaconFlood::doOpen() {
	beaconFlood_.messages_ = plainTextEdit_->toPlainText().split("\n");
	channelHop_.intfName_ = beaconFlood_.intfName_;
	channelHop_.channelList_ = QStringList(QString::number(beaconFlood_.channel_));

	return GGraph::doOpen();
}

bool BeaconFlood::doClose() {
	return GGraph::doClose();
}

void BeaconFlood::propLoad(QJsonObject jo) {
	GProp::propLoad(jo);
}

void BeaconFlood::propSave(QJsonObject& jo) {
	GProp::propSave(jo);
}

