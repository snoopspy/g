#pragma once

#include "gdot11exthdr.h"

// ----------------------------------------------------------------------------
// GBeaconHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GBeaconHdr : GDot11ExtHdr {
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

		void* value() {
			return (char*)this + sizeof(Tag);
		}

		Tag* next() {
			char* res = (char*)this;
			res += sizeof(Tag) + this->len_;
			return PTag(res);
		}
	};
	typedef Tag *PTag;

	Tag* firstTag() {
		char* p = pchar(this);
		p += sizeof(GBeaconHdr);
		return PTag(p);
	}

	// tagged parameter number
	enum: le8_t {
		TagSsidParameterSet = 0,
		TagSupportedRated = 1,
		TagDsParameterSet = 3,
		TagTrafficIndicationMap = 5,
		TagCountryInformation = 7,
		TagQbssLoadElement = 11,
		TagRsnInformation = 48,
		TagVendorSpecific = 221
	};

	struct TrafficIndicationMap : Tag {
		le8_t count_;
		le8_t period_;
		le8_t control_;
		le8_t bitmap_;
	};
	typedef TrafficIndicationMap *PTrafficIndicationMap;

	static GBeaconHdr* check(GDot11Hdr* dot11Hdr, uint32_t size);
	void* firstTag(le8_t num, uint32_t size);
};
typedef GBeaconHdr *PBeaconHdr;
#pragma pack(pop)
