#pragma once

#include "gdot11addrhdr.h"

// ----------------------------------------------------------------------------
// GQoSNullHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GQoSNullHdr : GDot11AddrHdr {
	GMac da() { return addr3_; }
	GMac sa() { return addr2_; }
	GMac bssid() { return addr1_; }
	GMac sta() { return addr2_; }

	le16_t qosControl_;

	static GQoSNullHdr* check(GDot11Hdr* dot11Hdr, uint32_t size);
};
typedef GQoSNullHdr *PQosNullHdr;
#pragma pack(pop)
