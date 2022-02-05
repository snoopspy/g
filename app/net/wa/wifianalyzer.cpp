#include "wifianalyzer.h"
#include <GBeaconHdr>

WifiAnalyzer::WifiAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -c1\"", "su -c \"nexutil -d\"", "su -c \"nexutil -m2\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	dot11Block_.writer_ = &pcapDeviceWrite_;
	pcapDeviceWrite_.mtu_ = 0;

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

	if (!monitorDevice_.open()) {
		err = monitorDevice_.err;
		return false;
	}

	channelHop_.intfName_ = monitorDevice_.intfName_;
	if (!channelHop_.open()) {
		err = channelHop_.err;
		return false;
	}

	if (!dot11Block_.open()) {
		err = dot11Block_.err;
		return false;
	}

	pcapDeviceWrite_.intfName_ = monitorDevice_.intfName_;
	if (!pcapDeviceWrite_.open()) {
		err = pcapDeviceWrite_.err;
		return false;
	}

    GThreadMgr::resumeStart();

	return true;
}

bool WifiAnalyzer::doClose() {
	monitorDevice_.close();
	channelHop_.close();
	dot11Block_.close();
	pcapDeviceWrite_.close();
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
	void* end = packet->buf_.data_ + packet->buf_.size_;
	while (tag < end) {
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
		if (tag == nullptr) break;
	}
	if (ssid == "") return;
	if (channel != 0 && ignoreOtherChannelFrame_ && currentChannel_ != channel)
		return;


	//
	// channel
	//
	GRadioHdr* radioHdr = packet->radioHdr_;
	Q_ASSERT(radioHdr != nullptr);
	if (channel == 0) {
		qWarning() << QString("can not find channel tag for %1").arg(ssid);
		QList<GBuf> freqList = radioHdr->presentInfo(GRadioHdr::Channel);
		if (freqList.count() > 0) {
			int16_t freq = *reinterpret_cast<uint16_t*>(freqList[0].data_);
			channel = GRadioHdr::freqToChannel(freq);
		}
	}

	//
	// signal
	//
	QList<GBuf> signalList = radioHdr->presentInfo(GRadioHdr::AntennaSignal);
	if (signalList.count() > 0) {
		signal = *pchar(signalList[0].data_);
		if (signal < minSignal_) return;
	} else
		return;

	emit detected(mac, ssid, channel, signal);
}

void WifiAnalyzer::processChannelChanged(int channel) {
	currentChannel_ = channel;
}
