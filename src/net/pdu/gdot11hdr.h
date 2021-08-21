#pragma once

#include "gradiotaphdr.h"

#pragma pack(push, 1)
struct GDot11Hdr {
	le8_t ver_:2;
	le8_t type_:2;
	le8_t subtype_:4;
	le8_t flags_;
	le16_t duration_;

	le8_t typeSubtype() { return type_ << 4 | subtype_; }

	// type
	enum: le8_t {
		Manage = 0,
		Control = 1,
		Data = 2
	};

	// typeSubtype
	enum: le8_t {
		Beacon = 0x08,
		QosNull = 0x2C,
		Acknowledgement = 0x1D
	};

	static GDot11Hdr* check(GRadiotapHdr* radiotapHdr, uint32_t size);
};
typedef GDot11Hdr *PDot11Hdr;
#pragma pack(pop)
