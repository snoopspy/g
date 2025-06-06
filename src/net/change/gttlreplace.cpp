#include "gttlreplace.h"

// ----------------------------------------------------------------------------
// GTtlReplace
// ----------------------------------------------------------------------------
void GTtlReplace::replace(GPacket* packet) {
	if (!enabled_) return;

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) {
		emit notReplaced(packet);
		return;
	}

	if (log_)
		qInfo() << QString("replaced %1 > %2").arg(ipHdr->ttl_).arg(ttl_);

	ipHdr->ttl_ = ttl_;
	ipHdr->sum_ = htons(GIpHdr::calcSum(ipHdr));

	packet->ctrl_.changed_ = true;
	emit replaced(packet);
}
