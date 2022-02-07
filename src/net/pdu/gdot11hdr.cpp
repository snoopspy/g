#include "gdot11hdr.h"

// ----------------------------------------------------------------------------
// GDot11Hdr
// ----------------------------------------------------------------------------
GDot11Hdr* GDot11Hdr::check(GRadioHdr* radioHdr, uint32_t size) {
	uint32_t len = radioHdr->len_;
	if (size < sizeof(GDot11Hdr) + len) {
		return nullptr;
	}
	GDot11Hdr* dot11ExtHdr = PDot11ExtHdr(pchar(radioHdr) + len);
	return dot11ExtHdr;
}

void GDot11Hdr::init(GMac addr1, GMac addr2, GMac addr3) {
	addr1_ = addr1;
	addr2_ = addr2;
	addr3_ = addr3;
	frag_ = 0;
	seq_ = 0;
}
