#pragma once

#include "gdot11hdr.h"

// ----------------------------------------------------------------------------
// GDot11AddrHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GDot11AddrHdr : GDot11Hdr {
	GMac addr1_;
	GMac addr2_;
	GMac addr3_;
	le8_t frag_:4;
	le16_t seq_:12;

	GMac ra() { return addr1_; }
	GMac ta() { return addr2_; }
};
typedef GDot11AddrHdr *PDot11AddrHdr;
#pragma pack(pop)
