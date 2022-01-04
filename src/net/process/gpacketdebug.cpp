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

	GRadiotapHdr* radiotapHdr = packet->radiotapHdr_;
	if (radiotapHdr != nullptr) {
		QList<GBuf> ba = radiotapHdr->getInfo(GRadiotapHdr::AntennaSignal);
		if (ba.count() > 0) {
			int8_t signal = *pchar(ba[0].data_);
			msg += " " + QString::number(signal) + " dBm";
		}

		GDot11Hdr* dot11Hdr = packet->dot11Hdr_;
		le8_t typeSubtype = dot11Hdr->typeSubtype();
		msg += " Ox" + QString::number(typeSubtype, 16);
		switch (typeSubtype) {
			case GDot11Hdr::AssociationRequest: msg += " AssociationRequest"; break;
			case GDot11Hdr::AssociationResponse: msg += " AssociationResponse"; break;
			case GDot11Hdr::ReassociationRequest: msg += " ReassociationRequest"; break;
			case GDot11Hdr::ReassociationResponse: msg += " ReassociationResponse"; break;
			case GDot11Hdr::ProbeRequest: msg += " ProbeRequest"; break;
			case GDot11Hdr::ProbeResponse: msg += " ProbeResponse"; break;
			case GDot11Hdr::TimingAdvertisement: msg += " TimingAdvertisement"; break;
			case GDot11Hdr::Reserved: msg += " Reserved"; break;
			case GDot11Hdr::Beacon: msg += " Beacon"; break;
			case GDot11Hdr::Atim: msg += " Atim"; break;
			case GDot11Hdr::Disassociation: msg += " Disassociation"; break;
			case GDot11Hdr::Authentication: msg += " Authentication"; break;
			case GDot11Hdr::Deauthentication: msg += " Deauthentication"; break;
			case GDot11Hdr::Action: msg += " Action"; break;
			case GDot11Hdr::ActionNoAck: msg += " ActionNoAck"; break;
			case GDot11Hdr::Trigger: msg += " Trigger"; break;
			case GDot11Hdr::Tack: msg += " Tack"; break;
			case GDot11Hdr::BeamformingReportPoll: msg += " BeamformingReportPoll"; break;
			case GDot11Hdr::VhtHeNdpAnnouncement: msg += " VhtHeNdpAnnouncement"; break;
			case GDot11Hdr::ControlFrameExtension: msg += " ControlFrameExtension"; break;
			case GDot11Hdr::ControlWrapper: msg += " ControlWrapper"; break;
			case GDot11Hdr::BlockAckRequest: msg += " BlockAckRequest"; break;
			case GDot11Hdr::BlockAck: msg += " BlockAck"; break;
			case GDot11Hdr::PsPoll: msg += " PsPoll"; break;
			case GDot11Hdr::Rts: msg += " Rts"; break;
			case GDot11Hdr::Cts: msg += " Cts"; break;
			case GDot11Hdr::Ack: msg += " Ack"; break;
			case GDot11Hdr::CfEnd: msg += " CfEnd"; break;
			case GDot11Hdr::CfEndCfACK: msg += " CfEndCfACK"; break;
			case GDot11Hdr::Data: msg += " Data"; break;
			case GDot11Hdr::Null: msg += " Null"; break;
			case GDot11Hdr::QoSData: msg += " QoSData"; break;
			case GDot11Hdr::QoSDataCfACK: msg += " QoSDataCfACK"; break;
			case GDot11Hdr::QoSDataCfPoll: msg += " QoSDataCfPoll"; break;
			case GDot11Hdr::QoSDataCfACKCfPoll: msg += " QoSDataCfACKCfPoll"; break;
			case GDot11Hdr::QoSNull: msg += " QoSNull"; break;
			case GDot11Hdr::QoSCfPoll: msg += " QoSCfPoll"; break;
			case GDot11Hdr::QoSCfACKCfPoll: msg += " QoSCfACKCfPoll"; break;
			case GDot11Hdr::DmgBeacon: msg += " DmgBeacon"; break;
			case GDot11Hdr::S1gBeacon: msg += " S1gBeacon"; break;
			default: qDebug() << "******** unknown typeSybtype *****"; break;
		}
	}

	msg = QString("%1:").arg(packet->buf_.size_) + msg;
	qDebug() << msg;

	emit debugged(packet);
}
