#include "wifijammer.h"
#include <GBeaconHdr>

WifiJammer::WifiJammer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m2\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	// for probeDetected signal
	qRegisterMetaType<GMac>("GMac");
	QObject::connect(&monitorDevice_, &GMonitorDevice::captured, this, &WifiJammer::processCaptured, Qt::DirectConnection);
}

WifiJammer::~WifiJammer() {
	close();
}

bool WifiJammer::doOpen() {
	if (!monitorDevice_.open()) {
		err = monitorDevice_.err;
		return false;
	}

	return true;
}

bool WifiJammer::doClose() {
	monitorDevice_.close();
	return true;
}

void WifiJammer::processCaptured(GPacket* packet) {
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

	emit attacked(mac, ssid);
}
