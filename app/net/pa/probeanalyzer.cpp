#include "probeanalyzer.h"

ProbeAnalyzer::ProbeAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -c1\"", "su -c \"nexutil -d\"", "su -c \"nexutil -m2\"", "su -c \"nexutil -k1\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\"", "su -c \"ifconfig wlan0 down\""}));
#endif

	// for probeDetected signal
	qRegisterMetaType<GMac>("GMac");
	QObject::connect(&monitorDevice_, &GMonitorDevice::captured, this, &ProbeAnalyzer::processCaptured, Qt::DirectConnection);
}

ProbeAnalyzer::~ProbeAnalyzer() {
	close();
}

bool ProbeAnalyzer::doOpen() {
	if (!monitorDevice_.open()) {
		err = monitorDevice_.err;
		return false;
	}

	return true;
}

bool ProbeAnalyzer::doClose() {
	monitorDevice_.close();
	return true;
}

void ProbeAnalyzer::processCaptured(GPacket* packet) {
	GDot11Hdr* dot11Hdr = packet->dot11Hdr_;
	if (dot11Hdr == nullptr) return;

	le8_t typeSubtype = dot11Hdr->typeSubtype();
	if (typeSubtype != GDot11::ProbeRequest && typeSubtype != GDot11::Deauthentication && typeSubtype != GDot11::Disassociation) return;

	GRadioHdr* radioHdr = packet->radioHdr_;
	if (radioHdr == nullptr) return;

	int8_t signal = radioHdr->getSignal();
	if (signal == 0) return;
	if (signal < minSignal_) return;

	GMac mac = dot11Hdr->ta();

	emit probeDetected(mac, signal);
}

