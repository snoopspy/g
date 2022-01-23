#include "wifianalyzer.h"
#include <GBeaconHdr>

WifiAnalyzer::WifiAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
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

	GBeaconHdr::Tag* tag = beaconHdr->getTag();
	gbyte* end = packet->buf_.data_ + packet->buf_.size_;
	QString ssid;
	while (true) {
		if (pbyte(tag) >= end) break;
		le8_t num = tag->num_;
		if (num == GBeaconHdr::TagSsidParameterSet) {
			const char* p = pchar(tag->value());
			ssid = QByteArray(p, tag->len_);
			break;
		}
		tag = tag->next();
		if (tag == nullptr) break;
	}
	if (ssid == "") return;

	GMac mac = dot11ExtHdr->ta();

	GRadiotapHdr* radiotapHdr = packet->radiotapHdr_;
	Q_ASSERT(radiotapHdr != nullptr);
	QList<GBuf> signalList = radiotapHdr->getInfo(GRadiotapHdr::AntennaSignal);
	if (signalList.count() == 0) return;
	qint8 signal = *pchar(signalList[0].data_);
	if (signal < minSignal_) return;

	emit detected(mac, ssid, signal);
}

