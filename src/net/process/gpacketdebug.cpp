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

				GIcmpHdr* icmpHdr = packet->icmpHdr_;
				if (icmpHdr != nullptr)
					msg += " icmp";
			}
		}
	}

	GRadiotapHdr* radiotapHdr = packet->radiotapHdr_;
	if (radiotapHdr != nullptr) {
		GDot11Hdr* dot11Hdr = packet->dot11Hdr_;
		switch (dot11Hdr->typeSubtype()) {
			case GDot11Hdr::Beacon: msg += " Beacon"; break;
			case GDot11Hdr::QosNull: msg += " QosNull"; break;
			case GDot11Hdr::Acknowledgement: msg += " Acknowledgement"; break;
		}
	}

	msg = QString("%1:").arg(packet->buf_.size_) + msg;
	qDebug() << msg;

	emit debugged(packet);
}
