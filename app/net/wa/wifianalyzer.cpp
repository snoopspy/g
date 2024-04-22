#include "wifianalyzer.h"
#include <GIw>

WifiAnalyzer::WifiAnalyzer(QObject* parent) : GStateObj(parent) {
	monitorDevice_.setObjectName("monitorDevice_");
	channelHop_.setObjectName("channelHop_");
	dot11Block_.setObjectName("dot11Block_");
	dot11Block_.setObjectName("dot11Block_");
	monitorDeviceWrite_.setObjectName("monitorDeviceWrite_");
	command_.setObjectName("command_");

#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -c1\"", "su -c \"nexutil -d\"", "su -c \"nexutil -m2\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\"", "su -c \"ifconfig wlan0 down\""}));
#endif

	dot11Block_.enabled_ = false;
	dot11Block_.writer_ = &monitorDeviceWrite_;

	// for probeDetected signal
	qRegisterMetaType<GMac>("GMac");
	QObject::connect(&monitorDevice_, &GMonitorDevice::captured, this, &WifiAnalyzer::processCaptured, Qt::DirectConnection);
	QObject::connect(&monitorDevice_, &GMonitorDevice::captured, &dot11Block_, &GDot11Block::block, Qt::DirectConnection);
	QObject::connect(&channelHop_, &GChannelHop::channelChanged, this, &WifiAnalyzer::processChannelChanged, Qt::DirectConnection);
}

WifiAnalyzer::~WifiAnalyzer() {
	close();
}

bool WifiAnalyzer::doOpen() {
	GThreadMgr::suspendStart();

	currentChannel_ = 0;
	bool isOk = false;
	while (true) {
		if (!monitorDevice_.open()) {
			err = monitorDevice_.err;
			break;
		}

		channelHop_.intfName_ = monitorDevice_.intfName_;
		if (!channelHop_.open()) {
			err = channelHop_.err;
			break;
		}

		if (!dot11Block_.open()) {
			err = dot11Block_.err;
			break;
		}

		monitorDeviceWrite_.intfName_ = monitorDevice_.intfName_;
		if (!monitorDeviceWrite_.open()) {
			err = monitorDeviceWrite_.err;
			break;
		}
		isOk = true;
		break;
	}

	GThreadMgr::resumeStart();
	return isOk;
}

bool WifiAnalyzer::doClose() {
	monitorDevice_.close();
	channelHop_.close();
	dot11Block_.close();
	monitorDeviceWrite_.close();
	return true;
}

void WifiAnalyzer::processCaptured(GPacket* packet) {
	GBeaconHdr* beaconHdr = packet->beaconHdr_;
	if (beaconHdr == nullptr) return;

	GMac mac;
	QString ssid;
	int channel = 0;
	int8_t signal = 127;

	//
	// mac
	//
	mac = beaconHdr->ta();

	//
	// ssid or channel
	//
	GBeaconHdr::Tag* tag = beaconHdr->firstTag();
	while (packet->buf_.contains(tag)) {
		le8_t num = tag->num_;
		switch (num) {
			case GBeaconHdr::TagSsidParameterSet: {
				ssid = QByteArray(pchar(tag->value()), tag->len_);
				break;
			}
			case GBeaconHdr::TagDsParameterSet: {
				char* p = pchar(tag->value());
				channel = *p;
				break;
			}
			default:
				break;
		}
		if (ssid != "" && channel != 0)
			break;
		tag = tag->next();
	}
	if (ssid == "" || channel == 0) return;
	if (ignoreOtherChannelFrame_ && currentChannel_ != channel)
		return;

	//
	// channel
	//
	GRadioHdr* radioHdr = packet->radioHdr_;
	Q_ASSERT(radioHdr != nullptr);
	if (channel == 0) {
		qWarning() << QString("can not find channel tag for %1").arg(ssid);
		QList<GBuf> freqList = radioHdr->getPresentFlags(GRadioHdr::Channel);
		if (freqList.count() > 0) {
			int16_t freq = *reinterpret_cast<uint16_t*>(freqList[0].data_);
			channel = GIw::freqToChannel(freq);
		}
	}

	//
	// signal
	//
	signal = radioHdr->getSignal();
	if (signal == 0) return;
	if (signal < minSignal_) return;

	emit detected(mac, ssid, channel, signal);
}

void WifiAnalyzer::processChannelChanged(int channel) {
	currentChannel_ = channel;
}
