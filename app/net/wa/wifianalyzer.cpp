#include "wifianalyzer.h"
#include <GBeaconHdr>

WifiAnalyzer::WifiAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -c1\""}));
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -d\""}));
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m2\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	// for probeDetected signal
	qRegisterMetaType<GMac>("GMac");
	QObject::connect(&monitorDevice_, &GMonitorDevice::captured, this, &WifiAnalyzer::processCaptured, Qt::DirectConnection);
}

WifiAnalyzer::~WifiAnalyzer() {
	close();
}

bool WifiAnalyzer::doOpen() {
	if (!monitorDevice_.open()) {
		err = monitorDevice_.err;
		return false;
	}

	channelHop_.intfName_ = monitorDevice_.intfName_;
	if (!channelHop_.open()) {
		err = channelHop_.err;
		return false;
	}

	return true;
}

bool WifiAnalyzer::doClose() {
	monitorDevice_.close();
	channelHop_.close();
	return true;
}

void WifiAnalyzer::processCaptured(GPacket* packet) {
	GDot11ExtHdr* dot11ExtHdr = packet->dot11ExtHdr_;
	if (dot11ExtHdr == nullptr) return;

	le8_t typeSubtype = dot11ExtHdr->typeSubtype();
	if (typeSubtype != GDot11Hdr::Beacon) return;

	GBeaconHdr* beaconHdr = GBeaconHdr::check(dot11ExtHdr, packet->buf_.size_);
	if (beaconHdr == nullptr) return;

	GMac mac;
	QString ssid;
	int8_t channel = -1;
	int8_t signal = 127;

	//
	// mac
	//
	mac = dot11ExtHdr->ta();

	//
	// ssid or channel
	//
	GBeaconHdr::Tag* tag = beaconHdr->getTag();
	gbyte* end = packet->buf_.data_ + packet->buf_.size_;
	while (true) {
		if (pbyte(tag) >= end) break;
		le8_t num = tag->num_;
		switch (num) {
			case GBeaconHdr::TagSsidParameterSet: {
				char* p = pchar(tag->value());
				ssid = QByteArray(p, tag->len_);
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
		if (ssid != "" && channel != -1)
			break;
		tag = tag->next();
		if (tag == nullptr) break;
	}
	if (ssid == "") return;

	//
	// channel
	//
	GRadiotapHdr* radiotapHdr = packet->radiotapHdr_;
	Q_ASSERT(radiotapHdr != nullptr);
	if (channel == -1) {
		qWarning() << QString("can not find channel tag for %1").arg(ssid);
		QList<GBuf> channelList = radiotapHdr->getInfo(GRadiotapHdr::Channel);
		if (channelList.count() > 0) {
			int16_t freq = *reinterpret_cast<uint16_t*>(channelList[0].data_);
			channel = GRadiotapHdr::freqToChannel(freq);
		}
	}

	//
	// signal
	//
	QList<GBuf> signalList = radiotapHdr->getInfo(GRadiotapHdr::AntennaSignal);
	if (signalList.count() > 0) {
		signal = *pchar(signalList[0].data_);
		if (signal < minSignal_) return;
	} else
		return;

	emit detected(mac, ssid, channel, signal);
}
