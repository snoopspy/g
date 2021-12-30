#include "gcorrectchecksum.h"

// ----------------------------------------------------------------------------
// GCorrectChecksum
// ----------------------------------------------------------------------------
void GCorrectChecksum::correct(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;
	bool changed = false;

	if (ipChecksum_) {
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
		changed = true;
	}

	if (tcpChecksum_) {
		GTcpHdr* tcpHdr = packet->tcpHdr_;
		if (tcpHdr != nullptr) {
			tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
			changed = true;
		}
	}

	if (udpChecksum_) {
		GUdpHdr* udpHdr = packet->udpHdr_;
		if (udpHdr != nullptr) {
			udpHdr->sum_ = htons(GUdpHdr::calcChecksum(ipHdr, udpHdr));
			changed = true;
		}
	}

	packet->ctrl_.changed_ = changed;
	if (changed)
		emit corrected(packet);
}
