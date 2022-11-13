#include "gauthhdr.h"

// ----------------------------------------------------------------------------
// GAuthHdr
// ----------------------------------------------------------------------------
GAuthHdr* GAuthHdr::check(GDot11* dot11Hdr, uint32_t size) {
	Q_ASSERT(dot11Hdr->typeSubtype() == GDot11::Authentication);
	if (size < sizeof(GAuthHdr)) {
		qWarning() << QString("invalid size %1").arg(size);
		return nullptr;
	}
	return PAuthHdr(dot11Hdr);
}

void GAuthHdr::init(GMac apMac, GMac staMac) {
	GDot11::init(GDot11::Authentication);
	GDot11Hdr::init(apMac, staMac, staMac);
	fix_.authAlgorithm_ = 0x0000;
	fix_.authSeq_ = 0x0001;
	fix_.statusCode_ = 0x0000;
}
