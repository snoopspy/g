#include "gpacketdebug.h"
#include <iostream>

QString dump(u_char* data, size_t size) {
	QString raw;
	QString hexa;
	if (size > 16) size = 16;
	while (size > 0) {
		char ch = char(*data);
		if (isprint(u_char(ch)))
			raw += ch;
		else
			raw += '.';

		char ch1 = (ch & 0xF0) >> 4;
		if (ch1 >= 10)
			ch1 += 'A' - 10;
		else
			ch1 += '0';
		char ch2 = (ch & 0x0F);
		if (ch2 >= 10)
			ch2 += 'A' - 10;
		else
			ch2 += '0';
		hexa += ch1;
		hexa += ch2;
		hexa += ' ';

		data++;
		size--;
	}
	return raw + " " + hexa;
}

// ----------------------------------------------------------------------------
// GPacketDebug
// ----------------------------------------------------------------------------
void GPacketDebug::debug(GPacket* packet) {
	if (!enabled_) return;

	QString msg;

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr != nullptr) {
		msg += QString(" eth %1>%2").arg(QString(ethHdr->smac()), QString(ethHdr->dmac()));

		GIpHdr* ipHdr = packet->ipHdr_;
		if (ipHdr != nullptr) {
			msg += QString(" ip %1>%2").arg(QString(ipHdr->sip()), QString(ipHdr->dip()));

			GTcpHdr* tcpHdr = packet->tcpHdr_;
			if (tcpHdr != nullptr) {
				msg += QString(" tcp %1>%2").arg(tcpHdr->sport()).arg(tcpHdr->dport());

				GBuf tcpData = packet->tcpData_;
				if (tcpData.valid())
					msg += " " + dump(tcpData.data_, tcpData.size_);
			}

			GUdpHdr* udpHdr = packet->udpHdr_;
			if (udpHdr != nullptr) {
				msg += QString(" udp %1>%2").arg(udpHdr->sport()).arg(udpHdr->dport());

				GBuf udpData = packet->udpData_;
				if (udpData.valid())
					msg += " " + dump(udpData.data_, udpData.size_);
			}

			GIcmpHdr* icmpHdr = packet->icmpHdr_;
			if (icmpHdr != nullptr)
				msg += " icmp";
		}
	}

	GRadioHdr* radioHdr = packet->radioHdr_;
	if (radioHdr != nullptr) {
		QList<GBuf> ba = radioHdr->presentInfo(GRadioHdr::AntennaSignal);
		if (ba.count() > 0) {
			int8_t signal = *pchar(ba[0].data_);
			msg += " " + QString::number(signal) + " dBm";
		}

		GDot11* dot11Hdr = packet->dot11_;
		le8_t typeSubtype = dot11Hdr->typeSubtype();
		msg += " Ox" + QString::number(typeSubtype, 16);
		switch (typeSubtype) {
			case GDot11::AssociationRequest: msg += " AssociationRequest"; break;
			case GDot11::AssociationResponse: msg += " AssociationResponse"; break;
			case GDot11::ReassociationRequest: msg += " ReassociationRequest"; break;
			case GDot11::ReassociationResponse: msg += " ReassociationResponse"; break;
			case GDot11::ProbeRequest: msg += " ProbeRequest"; break;
			case GDot11::ProbeResponse: msg += " ProbeResponse"; break;
			case GDot11::TimingAdvertisement: msg += " TimingAdvertisement"; break;
			case GDot11::Reserved: msg += " Reserved"; break;
			case GDot11::Beacon: msg += " Beacon"; break;
			case GDot11::Atim: msg += " Atim"; break;
			case GDot11::Disassociation: msg += " Disassociation"; break;
			case GDot11::Authentication: msg += " Authentication"; break;
			case GDot11::Deauthentication: msg += " Deauthentication"; break;
			case GDot11::Action: msg += " Action"; break;
			case GDot11::ActionNoAck: msg += " ActionNoAck"; break;
			case GDot11::Trigger: msg += " Trigger"; break;
			case GDot11::Tack: msg += " Tack"; break;
			case GDot11::BeamformingReportPoll: msg += " BeamformingReportPoll"; break;
			case GDot11::VhtHeNdpAnnouncement: msg += " VhtHeNdpAnnouncement"; break;
			case GDot11::ControlFrameExtension: msg += " ControlFrameExtension"; break;
			case GDot11::ControlWrapper: msg += " ControlWrapper"; break;
			case GDot11::BlockAckRequest: msg += " BlockAckRequest"; break;
			case GDot11::BlockAck: msg += " BlockAck"; break;
			case GDot11::PsPoll: msg += " PsPoll"; break;
			case GDot11::Rts: msg += " Rts"; break;
			case GDot11::Cts: msg += " Cts"; break;
			case GDot11::Ack: msg += " Ack"; break;
			case GDot11::CfEnd: msg += " CfEnd"; break;
			case GDot11::CfEndCfACK: msg += " CfEndCfACK"; break;
			case GDot11::Data: msg += " Data"; break;
			case GDot11::Null: msg += " Null"; break;
			case GDot11::QoSData: msg += " QoSData"; break;
			case GDot11::QoSDataCfACK: msg += " QoSDataCfACK"; break;
			case GDot11::QoSDataCfPoll: msg += " QoSDataCfPoll"; break;
			case GDot11::QoSDataCfACKCfPoll: msg += " QoSDataCfACKCfPoll"; break;
			case GDot11::QoSNull: msg += " QoSNull"; break;
			case GDot11::QoSCfPoll: msg += " QoSCfPoll"; break;
			case GDot11::QoSCfACKCfPoll: msg += " QoSCfACKCfPoll"; break;
			case GDot11::DmgBeacon: msg += " DmgBeacon"; break;
			case GDot11::S1gBeacon: msg += " S1gBeacon"; break;
			default: qDebug() << "******** unknown typeSybtype *****"; break;
		}
	}

	msg = QString("%1:").arg(packet->buf_.size_) + msg;
	qDebug() << msg;

	emit debugged(packet);
}
