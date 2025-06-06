#include "gcorrectchecksum.h"

// ----------------------------------------------------------------------------
// GCorrectChecksum
// ----------------------------------------------------------------------------
void GCorrectChecksum::correct(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;
	bool changed = false;

	if (ipChecksum_) {
		ipHdr->sum_ = GIpHdr::inetCalcSum(ipHdr);
		changed = true;
	}

	if (tcpChecksum_) {
		GTcpHdr* tcpHdr = packet->tcpHdr_;
		if (tcpHdr != nullptr) {
			tcpHdr->sum_ = GTcpHdr::inetCalcSum(ipHdr, tcpHdr);
			changed = true;
		}
	}

	if (udpChecksum_) {
		GUdpHdr* udpHdr = packet->udpHdr_;
		if (udpHdr != nullptr) {
			udpHdr->sum_ = GUdpHdr::inetCalcSum(ipHdr, udpHdr);
			changed = true;
		}
	}

	if (icmpChecksum_) {
		GIcmpHdr* icmpHdr = packet->icmpHdr_;
		if (icmpHdr != nullptr) {
			icmpHdr->sum_ = GIcmpHdr::inetCalcSum(ipHdr, icmpHdr);
			changed = true;
		}
	}

	packet->ctrl_.changed_ = changed;
	if (changed)
		emit corrected(packet);
}
