#pragma once

#include "gdot11.h"

// ----------------------------------------------------------------------------
// GDot11Hdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GDot11Hdr : GDot11 {
	GMac addr1_; // receiver address
	GMac addr2_; // transmit address
	GMac addr3_;
	le8_t frag_:4;
	le16_t seq_:12;

	GMac ra() { return addr1_; }
	GMac ta() { return addr2_; }

	static GDot11Hdr* check(GRadioHdr* radioHdr, uint32_t size);
	void init(GMac addr1, GMac addr2, GMac addr3);
};
typedef GDot11Hdr *PDot11ExtHdr;
#pragma pack(pop)
