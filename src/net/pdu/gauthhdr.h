#pragma once

#include "gdot11hdr.h"

// ----------------------------------------------------------------------------
// GAuthHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GAuthHdr : GDot11Hdr {
	GMac da() { return addr1_; }
	GMac sa() { return addr2_; }
	GMac bssid() { return addr3_; }

	struct Fix {
		le16_t authAlgorithm_;
		le16_t authSeq_;
		le16_t statusCode_;
	} fix_;

	static GAuthHdr* check(GDot11* dot11Hdr, uint32_t size);
	void init(GMac apMac, GMac staMac);
};
typedef GAuthHdr *PAuthHdr;
#pragma pack(pop)
