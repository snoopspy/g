#include "gqosnullhdr.h"

GQosNullHdr* GQosNullHdr::check(GDot11Hdr* dot11Hdr, uint32_t size) {
	assert(dot11Hdr->typeSubtype() == GDot11Hdr::QosNull);
	if (size < sizeof(GQosNullHdr)) {
		qWarning() << QString("invalid size %1").arg(size);
		// dump(puchar(dot11Hdr), size); // gilgil temp 2021.08.19
		return nullptr;
	}
	return PQosNullHdr(dot11Hdr);
}
