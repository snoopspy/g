#pragma once

#include "gdot11hdr.h"

// ----------------------------------------------------------------------------
// GDeauthHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GDeauthHdr : GDot11Hdr {
	GMac da() { return addr1_; }
	GMac sa() { return addr2_; }
	GMac bssid() { return addr3_; }

	struct Fix {
		le16_t reasonCode_;
	} fix_;

	// reason code
	enum: le16_t {
		ReceivedFromNonAssociatedSTA = 7
	};

	enum Direction {
		ApStation,
		StationAp
	};

	static GDeauthHdr* check(GDot11* dot11Hdr, uint32_t size);
	void init(GMac apMac); // AP broadcast
	void init(GMac apMac, GMac staMac, Direction direction);
};
typedef GDeauthHdr *PDeauthHdr;
#pragma pack(pop)
