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
	dot11_ = GDot11::check(radioHdr_, size);

	if (dot11_ == nullptr) return;
	dot11Hdr_ = GDot11Hdr::check(radioHdr_, size);

	size -= radioHdr_->len_;
	le8_t typeSubtype = dot11_->typeSubtype();
	switch(typeSubtype) {
		case GDot11::Beacon:
			beaconHdr_ = GBeaconHdr::check(dot11_, size);
			break;
		case GDot11::Deauthentication:
			deauthHdr_ = GDeauthHdr::check(dot11_, size);
			break;
		case GDot11::QoSNull:
			qosNullHdr_ = GQoSNullHdr::check(dot11_, size);
			break;
		default:
			break;
	}
#ifdef _DEBUG
	parsed_ = true;
#endif // _DEBUG
}
