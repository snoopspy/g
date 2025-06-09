#pragma once

#include "gdot11hdr.h"

// ----------------------------------------------------------------------------
// GBeaconHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GBeaconHdr : GDot11Hdr {
	GMac da() { return addr1_; }
	GMac sa() { return addr2_; }
	GMac bssid() { return addr3_; }

	struct Fix {
		le64_t timestamp_; // microsecond
		le16_t beaconInterval_; // millisecond
		le16_t capabilities_;
	} fix_;

	//
	// Tag
	//
	struct Tag {
		le8_t num_;
		le8_t len_;

		void* value() {
			return pbyte(this) + sizeof(Tag);
		}

		Tag* next() {
			gbyte* res = pbyte(this);
			res += sizeof(Tag) + this->len_;
			return PTag(res);
		}
	};
	typedef Tag *PTag;

	Tag* firstTag() {
		gbyte* p = pbyte(this);
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
		TagHtCapabilities = 45,
		TagRsnInformation = 48,
		TagHtInformation = 61,
		TagVendorSpecific = 221
	};

	struct TrafficIndicationMap : Tag {
		le8_t count_;
		le8_t period_;
		le8_t control_;
		le8_t bitmap_;
	};
	typedef TrafficIndicationMap *PTrafficIndicationMap;

	struct HtCapabilities : Tag {
		le16_t capabilitiesInfo_;
		le8_t mpduParameters_;
		le8_t mcsSet_[16];
		le16_t extCapabilities_;
		le32_t txbfCapabilities_;
		le8_t aselCapabilities_;
	};
	typedef HtCapabilities *PHtCapabilities;

	struct HtInformation : Tag {
		le8_t primaryChannel_;
		le8_t htInformationSubset1_;
		le16_t htInformationSubset2_;
		le16_t htInformationSubset3_;
		le8_t basicMcsSet_[16];
	};
	typedef HtInformation *PHtInformation;

	static GBeaconHdr* check(GDot11* dot11Hdr, uint32_t size);
	void* findFirstTag(le8_t num, uint32_t size);
};
typedef GBeaconHdr *PBeaconHdr;
#pragma pack(pop)
