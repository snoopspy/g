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
		ProbeRequest = 0x04,
		ProbeResponse = 0x05,
		Beacon = 0x08,
		BlockAck = 0x19,
		RequestToSend = 0x1B,
		ClearToSend = 0x1C,
		Acknowledgement = 0x1D,
		DataFrame = 0x20,
		NullFunction = 0x24,
		QosData = 0x28,
		QosNull = 0x2C
	};

	static GDot11Hdr* check(GRadiotapHdr* radiotapHdr, uint32_t size);
};
typedef GDot11Hdr *PDot11Hdr;
#pragma pack(pop)
