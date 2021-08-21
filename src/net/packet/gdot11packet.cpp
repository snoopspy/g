#include "gdot11packet.h"

// ----------------------------------------------------------------------------
// GDot11Packet
// ----------------------------------------------------------------------------
void GDot11Packet::parse() {
#ifdef _DEBUG
	if (parsed_) {
		qCritical() << "already parsed";
		return;
	}
#endif // _DEBUG
	size_t size = buf_.size_;
	radiotapHdr_ = GRadiotapHdr::check(buf_.data_, size);
	if (radiotapHdr_ != nullptr) {
		le16_t radiotapLen = radiotapHdr_->len_;
		size -= radiotapLen;
		if (!radiotapHdr_->isShortPreamble()) // gilgil temp 2021.08.19
			dot11Hdr_ = GDot11Hdr::check(radiotapHdr_, size);
	}
#ifdef _DEBUG
	parsed_ = true;
#endif // _DEBUG
}
