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
		dot11Hdr_ = GDot11Hdr::check(radiotapHdr_, size);
	}
#ifdef _DEBUG
	parsed_ = true;
#endif // _DEBUG
}
