#include "getharppacket.h"

void GEthArpPacket::init(GMac dmac, GMac smac, uint16_t operation, GMac senderMac, GIp senderIp, GMac targetMac, GIp targetIp) {
	ethHdr_.dmac_ = dmac;
	ethHdr_.smac_ = smac;
	ethHdr_.type_ = htons(GEthHdr::Arp);

	arpHdr_.hrd_ = htons(GArpHdr::Ether);
	arpHdr_.pro_ = htons(GEthHdr::Ip4);
	arpHdr_.hln_ = GMac::Size;
	arpHdr_.pln_ = GIp::Size;
	arpHdr_.op_ = htons(operation);
	arpHdr_.smac_ = senderMac;
	arpHdr_.sip_ = htonl(senderIp);
	arpHdr_.tmac_ = targetMac;
	arpHdr_.tip_ = htonl(targetIp);
}
