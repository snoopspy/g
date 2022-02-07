#pragma once

#include "gdot11hdr.h"

// ----------------------------------------------------------------------------
// GQoSNullHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GQoSNullHdr : GDot11Hdr {
	GMac da() { return addr3_; }
	GMac sa() { return addr2_; }
	GMac bssid() { return addr1_; }
	GMac sta() { return addr2_; }

	le16_t qosControl_;

	static GQoSNullHdr* check(GDot11* dot11Hdr, uint32_t size);
};
typedef GQoSNullHdr *PQosNullHdr;
#pragma pack(pop)
