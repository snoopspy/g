#include "gpacket.h"

// ----------------------------------------------------------------------------
// GPacket
// ----------------------------------------------------------------------------
QString GPacket::dltToString(Dlt dlt) {
	QString res;
	switch (dlt) {
		case GPacket::Eth:
			res = "GPacket::Eth";
			break;
		case GPacket::Ip:
			res = "GPacket::Ip";
			break;
		case GPacket::Dot11:
			res = "GPacket::Dot11";
			break;
		case GPacket::Null:
			res = "GPacket::Null";
			break;
	}
	return res;
}

int GPacket::dltToInt(Dlt dlt) {
	int res;
	switch (dlt) {
		case GPacket::Eth:
			res = DLT_EN10MB;
			break;
		case GPacket::Ip:
			res = DLT_RAW;
			break;
		case GPacket::Dot11:
			res = DLT_IEEE802_11_RADIO;
			break;
		case GPacket::Null:
		default:
			res = DLT_NULL;
			break;
	}
	return res;
}

GPacket::Dlt GPacket::intToDlt(int dataLink) {
	Dlt res;
	switch (dataLink) {
		case DLT_EN10MB:
			res = GPacket::Eth;
			break;
		case DLT_RAW:
		case DLT_IPV4:
		case DLT_IPV6:
			res = GPacket::Ip;
			break;
		case DLT_IEEE802_11_RADIO:
			res = GPacket::Dot11;
			break;
		case DLT_NULL:
		default:
			res = GPacket::Null;
			break;
	}
	return res;
}

GPacket::GPacket(const GPacket& r) : QObject(r.parent()) {
	operator =(r);
}

GPacket& GPacket::operator = (const GPacket& r) {
	dlt_ = r.dlt_;
	ts_ = r.ts_;
	buf_ = r.buf_;
	ctrl_.block_ = r.ctrl_.block_;
	ctrl_.changed_ = r.ctrl_.changed_;

	ethHdr_ = r.ethHdr_;
	arpHdr_ = r.arpHdr_;
	ipHdr_ = r.ipHdr_;
	ip6Hdr_ = r.ip6Hdr_;
	tcpHdr_ = r.tcpHdr_;
	udpHdr_ = r.udpHdr_;
	icmpHdr_ = r.icmpHdr_;
	tcpData_ = r.tcpData_;
	udpData_ = r.udpData_;

#ifdef _DEBUG
	parsed_ = r.parsed_;
#endif // _DEBUG

	return *this;
}

GPacket::operator QString() const {
	QString smac;
	QString dmac;
	GIp sip(0);
	GIp dip(0);
	QString protoStr = "unknown";
	uint16_t sport = 0;
	uint16_t dport = 0;

	if (ethHdr_ != nullptr) {
		smac = QString(ethHdr_->smac());
		dmac = QString(ethHdr_->dmac());
	}

	if (ipHdr_ != nullptr) {
		sip = ipHdr_->sip();
		dip = ipHdr_->dip();
		uint8_t proto = ipHdr_->p();
		if (proto == GIpHdr::Tcp) {
			protoStr = "TCP";
			Q_ASSERT(tcpHdr_ != nullptr);
			sport = tcpHdr_->sport();
			dport = tcpHdr_->dport();
		}
		if (proto == GIpHdr::Udp) {
			protoStr = "UDP";
			Q_ASSERT(udpHdr_ != nullptr);
			sport = udpHdr_->sport();
			dport = udpHdr_->dport();
		}
	}

	QString msg = QString("%1 %2:%3 > %4:%5").arg(protoStr, QString(sip), QString::number(sport), QString(dip), QString::number(dport));

	if (smac != "")
		msg = QString("%1 > %2 %3").arg(smac, dmac, msg);

	return msg;
}

void GPacket::parse() {
	qCritical() << "virtual function call";
}

void GPacket::copyFrom(GPacket* source, GBuf newBuf) {
	clear();
	ts_ = source->ts_;
	buf_ = newBuf;
	ctrl_ = source->ctrl_;

	parse();
}
