#pragma once

#include "gdot11hdr.h"

#pragma pack(push, 1)
struct GBeaconHdr : GDot11Hdr {
	GMac addr1_;
	GMac addr2_;
	GMac addr3_;
	le8_t frag_:4;
	le16_t seq_:12;

	GMac ra() { return addr1_;}
	GMac da() { return addr1_; }
	GMac ta() { return addr2_; }
	GMac sa() { return addr2_; }
	GMac bssid() { return addr3_; }

	struct __attribute__((packed)) Fix {
		le64_t timestamp_; // microsecond
		le16_t beaconInterval_; // millisecond
		le16_t capabilities_;
	} fix_;

	struct Tag {
		le8_t num_;
		le8_t len_;
		Tag* next() {
			char* res = (char*)this;
			res += sizeof(Tag) + this->len_;
			return PTag(res);
		}
	};
	typedef Tag *PTag;
	Tag* tag() {
		char* p = pchar(this);
		p += sizeof(GBeaconHdr);
		return PTag(p);
	}

	// tagged parameter number
	enum: le8_t {
		tagSsidParameterSet = 0,
		tagSupportedRated = 1,
		tagTrafficIndicationMap = 5
	};

	struct TrafficIndicationMap : Tag {
		le8_t count_;
		le8_t period_;
		le8_t control_;
		le8_t bitmap_;
	};
	typedef TrafficIndicationMap *PTrafficIndicationMap;

	static GBeaconHdr* check(GDot11Hdr* dot11Hdr, uint32_t size);
	TrafficIndicationMap* getTim(uint32_t size);
};
typedef GBeaconHdr *PBeaconHdr;
#pragma pack(pop)
