#include "probeanalyzer.h"

ProbeAnalyzer::ProbeAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.clear();
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m2\""}));
	command_.closeCommands_.clear();
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	QObject::connect(&monitorDevice_, &GMonitorDevice::captured, this, &ProbeAnalyzer::processCaptured, Qt::DirectConnection);
}

ProbeAnalyzer::~ProbeAnalyzer() {
	close();
}

bool ProbeAnalyzer::doOpen() {
	if (!command_.open()) {
		err = command_.err;
		return false;
	}

	if (!monitorDevice_.open()) {
		err = monitorDevice_.err;
		return false;
	}

	return true;
}

bool ProbeAnalyzer::doClose() {
	monitorDevice_.close();
	command_.close();
	return true;
}

void ProbeAnalyzer::processCaptured(GPacket* packet) {
	GDot11ExtHdr* dot11ExtHdr = packet->dot11ExtHdr_;
	if (dot11ExtHdr == nullptr) return;

	le8_t typeSubtype = dot11ExtHdr->typeSubtype();
	if (typeSubtype != GDot11Hdr::ProbeRequest && typeSubtype != GDot11Hdr::Deauthentication) return;

	GRadiotapHdr* radiotapHdr = packet->radiotapHdr_;
	if (radiotapHdr == nullptr) return;

	QList<GBuf> signalList = radiotapHdr->getInfo(GRadiotapHdr::AntennaSignal);
	if (signalList.count() == 0) return;

	int8_t signal = *pchar(signalList[0].data_);
	if (signal < minSignal_) return;

	GMac mac = dot11ExtHdr->ta();

	emit probeDetected(mac, signal);
}
