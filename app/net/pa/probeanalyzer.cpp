#include "probeanalyzer.h"
#include <GIw>

ProbeAnalyzer::ProbeAnalyzer(QObject* parent) : GGraph(parent) {
	monitorDevice_.setObjectName("monitorDevice_");
	command_.setObjectName("command_");

#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -c1\"", "su -c \"nexutil -d\"", "su -c \"nexutil -m2\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\"", "su -c \"ifconfig wlan0 down\""}));
#endif

	nodes_.append(&monitorDevice_);

	// for probeDetected signal
	qRegisterMetaType<GMac>("GMac");
	QObject::connect(&monitorDevice_, &GMonitorDevice::captured, this, &ProbeAnalyzer::processCaptured, Qt::DirectConnection);
}

ProbeAnalyzer::~ProbeAnalyzer() {
	close();
}

void ProbeAnalyzer::processCaptured(GPacket* packet) {
	GDot11Hdr* dot11Hdr = packet->dot11Hdr_;
	if (dot11Hdr == nullptr) return;

	QString type = "";
	le8_t typeSubtype = dot11Hdr->typeSubtype();
	switch (typeSubtype) {
		case GDot11::ProbeRequest: type = "PR"; break;
		case GDot11::Authentication: type = "AT"; break;
		case GDot11::AssociationRequest: type = "AS"; break;
		case GDot11::ReassociationRequest: type = "RA"; break;
		case GDot11::Deauthentication: type = "DT"; break;
		case GDot11::Disassociation: type = "DA"; break;
	}
	if (type == "") return;

	int channel = 0;
	QList<GBuf> frequencyList = packet->radioHdr_->getPresentFlags(GRadioHdr::Channel);
	if (frequencyList.size() > 0) {
		le16_t frequency = *reinterpret_cast<le16_t*>(frequencyList.at(0).data_);
		channel = GIw::freqToChannel(frequency);
	}

	GRadioHdr* radioHdr = packet->radioHdr_;
	if (radioHdr == nullptr) return;

	int8_t signal = radioHdr->getSignal();
	if (signal == 0) return;
	if (signal < minSignal_) return;

	GMac mac = dot11Hdr->ta();

	emit probeDetected(mac, type, channel, signal);
}

void ProbeAnalyzer::propLoad(QJsonObject jo) {
	GProp::propLoad(jo);
}

void ProbeAnalyzer::propSave(QJsonObject& jo) {
	GProp::propSave(jo);
}

