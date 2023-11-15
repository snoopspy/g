#include "gippacket.h"

// ----------------------------------------------------------------------------
// GIpPacket
// ----------------------------------------------------------------------------
void GIpPacket::parse() {
#ifdef _DEBUG
	if (parsed_) {
		qCritical() << "already parsed";
		return;
	}
#endif // _DEBUG
	gbyte* p = buf_.data_;
	uint8_t proto;
	switch (*p & 0xF0) {
		case 0x40: // version 4
			ipHdr_ = PIpHdr(p);
			proto = ipHdr_->p();
			p += ipHdr_->hlen() * 4;
			if (buf_.size_ < ipHdr_->tlen())
				qWarning() << QString("buf size(%1) is less than ip total len(%2)").arg(buf_.size_, ipHdr_->tlen());
			break;
		case 0x60: // version 6
			ip6Hdr_= PIp6Hdr(p);
			proto = ip6Hdr_->nh();
			p += sizeof(GIp6Hdr);
			if (buf_.size_ < sizeof(GIp6Hdr) + ip6Hdr_->plen())
				qWarning() << QString("buf size(%1) is less than ip6 payload len(%2)").arg(buf_.size_, ip6Hdr_->plen());
			break;
		default:
			qWarning() << QString("invalid ip header version(0x%1)").arg(QString::number(*p, 16));
			proto = 0; // unknown
			break;
	}

	if (ipHdr_ != nullptr) {
		switch (proto) {
			case GIpHdr::Tcp: // Tcp
				tcpHdr_ = PTcpHdr(p);
				tcpData_ = GTcpHdr::parseData(ipHdr_, tcpHdr_);
				break;
			case GIpHdr::Udp: // Udp
				udpHdr_ = PUdpHdr(p);
				udpData_ = GUdpHdr::parseData(udpHdr_);
				break;
			case GIpHdr::Icmp: // Icmp
				icmpHdr_ = PIcmpHdr(p);
				break;
			case GIpHdr::Igmp:
				igmpHdr_ = PIgmpHdr(p);
				break;
			default:
				qWarning() << QString("unknown protocol(%1)").arg(proto);
				break;
		}
	}
#ifdef _DEBUG
	parsed_ = true;
#endif // _DEBUG
}
