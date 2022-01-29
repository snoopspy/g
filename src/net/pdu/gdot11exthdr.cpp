#include "gdot11exthdr.h"

// ----------------------------------------------------------------------------
// GDot11ExtHdr
// ----------------------------------------------------------------------------
GDot11ExtHdr* GDot11ExtHdr::check(GRadiotapHdr* radiotapHdr, uint32_t size) {
	uint32_t len = radiotapHdr->len_;
	if (size < sizeof(GDot11ExtHdr) + len) {
		return nullptr;
	}
	GDot11ExtHdr* dot11ExtHdr = PDot11ExtHdr(pchar(radiotapHdr) + len);
	return dot11ExtHdr;
}

void GDot11ExtHdr::init(GMac addr1, GMac addr2, GMac addr3) {
	addr1_ = addr1;
	addr2_ = addr2;
	addr3_ = addr3;
	frag_ = 0;
	seq_ = 0;
}
