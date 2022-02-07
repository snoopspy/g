#include "gdeauthhdr.h"

// ----------------------------------------------------------------------------
// GDeauthHdr
// ----------------------------------------------------------------------------
GDeauthHdr* GDeauthHdr::check(GDot11* dot11Hdr, uint32_t size) {
	Q_ASSERT(dot11Hdr->typeSubtype() == GDot11::Deauthentication);
	if (size < sizeof(GDeauthHdr)) {
		qWarning() << QString("invalid size %1").arg(size);
		return nullptr;
	}
	return PDeauthHdr(dot11Hdr);
}

void GDeauthHdr::init(GMac apMac) {
	GDot11::init(GDot11::Deauthentication);
	GDot11Hdr::init(GMac::broadcastMac(), apMac, apMac);
	fix_.reasonCode_ = ReceivedFromNonAssociatedSTA;
}

void GDeauthHdr::init(GMac apMac, GMac staMac, Direction direction) {
	(void)apMac;
	(void)staMac;
	(void)direction;
	qWarning() << "not supported yet";
}
