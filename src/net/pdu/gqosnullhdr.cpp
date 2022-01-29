#include "gqosnullhdr.h"

// ----------------------------------------------------------------------------
// GQoSNullHdr
// ----------------------------------------------------------------------------
GQoSNullHdr* GQoSNullHdr::check(GDot11Hdr* dot11Hdr, uint32_t size) {
	Q_ASSERT(dot11Hdr->typeSubtype() == GDot11Hdr::QoSNull);
	if (size < sizeof(GQoSNullHdr)) {
		qWarning() << QString("invalid size %1").arg(size);
		// dump(puchar(dot11Hdr), size); // gilgil temp 2021.08.19
		return nullptr;
	}
	return PQosNullHdr(dot11Hdr);
}
