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
	radioHdr_ = GRadioHdr::check(buf_.data_, size);
	if (radioHdr_ == nullptr) return;
	dot11Hdr_ = GDot11Hdr::check(radioHdr_, size);
	if (dot11Hdr_ == nullptr) return;
	dot11ExtHdr_ = GDot11ExtHdr::check(radioHdr_, size);
#ifdef _DEBUG
	parsed_ = true;
#endif // _DEBUG
}
