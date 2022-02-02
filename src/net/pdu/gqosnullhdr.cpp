#include "gqosnullhdr.h"

// ----------------------------------------------------------------------------
// GQoSNullHdr
// ----------------------------------------------------------------------------
GQoSNullHdr* GQoSNullHdr::check(GDot11Hdr* dot11Hdr, uint32_t size) {
	Q_ASSERT(dot11Hdr->typeSubtype() == GDot11Hdr::QoSNull);
	if (size < sizeof(GQoSNullHdr)) {
		qWarning() << QString("invalid size %1").arg(size);
		return nullptr;
	}
	return PQosNullHdr(dot11Hdr);
}
