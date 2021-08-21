#pragma once

#include "gdot11hdr.h"

#pragma pack(push, 1)
struct GQosNullHdr : GDot11Hdr {
	GMac addr1_;
	GMac addr2_;
	GMac addr3_;
	le8_t frag_:4;
	le16_t seq_:12;
	le16_t qosControl_;

	GMac ra() { return addr1_; }
	GMac ta() { return addr2_; }
	GMac da() { return addr3_; }
	GMac sa() { return addr2_; }
	GMac bssid() { return addr1_; }
	GMac sta() { return addr2_; }

	static GQosNullHdr* check(GDot11Hdr* dot11Hdr, uint32_t size);
};
typedef GQosNullHdr *PQosNullHdr;
#pragma pack(pop)
