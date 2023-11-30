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
		qInfo() << QString("replaced(%1 > %2").arg(ipHdr->ttl_).arg(ttl_);

	uint16_t oldChecksum = ipHdr->sum();
	uint16_t oldValue = ipHdr->ttl_ << 8 | ipHdr->p_;
	uint16_t newValue = ttl_ << 8 | ipHdr->p_;
	ipHdr->sum_ = htons(GIpHdr::recalcChecksum(oldChecksum, oldValue, newValue));

	ipHdr->ttl_ = ttl_;
	packet->ctrl_.changed_ = true;
	emit replaced(packet);
}
