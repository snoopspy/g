#include "gbeaconhdr.h"

// ----------------------------------------------------------------------------
// GBeaconHdr
// ----------------------------------------------------------------------------
GBeaconHdr* GBeaconHdr::check(GDot11Hdr* dot11Hdr, uint32_t size) {
	Q_ASSERT(dot11Hdr->typeSubtype() == GDot11Hdr::Beacon);
	if (size < sizeof(GBeaconHdr)) {
		qWarning() << QString("invalid size %1").arg(size);
		return nullptr;
	}
	return PBeaconHdr(dot11Hdr);
}

void* GBeaconHdr::getTag(le8_t num, uint32_t size) {
	void* end = pchar(this) + size;
	GBeaconHdr::Tag* t = getTag();
	while (true) {
		if (t >= end) break;
		if (t->num_ == num) {
			void* res = pvoid(t);
			return res;
		}
		t = t->next();
	}
	return nullptr;
}

#ifdef GTEST
#include <gtest/gtest.h>

#include "gradiotaphdr.h"

TEST(BeaconHdr, typeTest) {
	uint8_t packet[] = { // dot11-sample.pcap frame.number==1
		 0x00, 0x00, 0x12, 0x00, 0x2e, 0x48, 0x00, 0x00, 0x00, 0x02, 0x6c, 0x09, 0xa0, 0x00, 0xe9, 0x01,
		 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x64, 0xe5, 0x99, 0x7a,
		 0xe9, 0x64, 0x64, 0xe5, 0x99, 0x7a, 0xe9, 0x64, 0x50, 0xd5, 0x52, 0x61, 0xd6, 0xdc, 0x00, 0x00,
		 0x00, 0x00, 0x64, 0x00, 0x11, 0x0c, 0x00, 0x07, 0x74, 0x65, 0x73, 0x74, 0x2d, 0x61, 0x70, 0x01,
		 0x04, 0x82, 0x84, 0x8b, 0x96, 0x03, 0x01, 0x01, 0x07, 0x06, 0x4b, 0x52, 0x20, 0x01, 0x0e, 0x14,
		 0x05, 0x04, 0x00, 0x01, 0x00, 0x00, 0xdd, 0x31, 0x00, 0x50, 0xf2, 0x04, 0x10, 0x4a, 0x00, 0x01,
		 0x10, 0x10, 0x44, 0x00, 0x01, 0x02, 0x10, 0x47, 0x00, 0x10, 0x28, 0x80, 0x28, 0x80, 0x28, 0x80,
		 0x18, 0x80, 0xa8, 0x80, 0x64, 0xe5, 0x99, 0x7a, 0xe9, 0x64, 0x10, 0x3c, 0x00, 0x01, 0x01, 0x10,
		 0x49, 0x00, 0x06, 0x00, 0x37, 0x2a, 0x00, 0x01, 0x20, 0x30, 0x14, 0x01, 0x00, 0x00, 0x0f, 0xac,
		 0x04, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00, 0xdd,
		 0x18, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0x80, 0x00, 0x03, 0xa4, 0x00, 0x00, 0x27, 0xa4, 0x00,
		 0x00, 0x42, 0x43, 0x5e, 0x00, 0x62, 0x32, 0x2f, 0x00, 0x0b, 0x05, 0x00, 0x00, 0x2c, 0x12, 0x7a,
		 0xdd, 0x07, 0x00, 0x0c, 0x43, 0x04, 0x00, 0x00, 00
	};

	GRadiotapHdr* radiotapHdr = GRadiotapHdr::check(packet, sizeof(packet));
	EXPECT_NE(radiotapHdr, nullptr);
	GBeaconHdr* beaconHdr = PBeaconHdr(packet + radiotapHdr->len_);

	le8_t typeSubtype = beaconHdr->typeSubtype();
	EXPECT_EQ(typeSubtype, GDot11Hdr::Beacon);

	le16_t duration = beaconHdr->duration_;
	EXPECT_EQ(duration, 0);

	le8_t frag = beaconHdr->frag_;
	EXPECT_EQ(frag, 0);

	le16_t seq = beaconHdr->seq_;
	EXPECT_EQ(seq, 3413);

	GBeaconHdr::Fix* fix = &beaconHdr->fix_;

	le64_t timestamp = fix->timestamp_;
	EXPECT_EQ(timestamp, 3705037138);

	le16_t beaconInterval = fix->beaconInterval_;
	EXPECT_EQ(beaconInterval, 100); // 100 msec

	le16_t capabilities = fix->capabilities_;
	EXPECT_EQ(capabilities, 0x0C11);

	GBeaconHdr::Tag* tag = beaconHdr->getTag();
	le8_t num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagSsidParameterSet);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagSupportedRated);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagDsParameterSet);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagCountryInformation);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagTrafficIndicationMap);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagVendorSpecific);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagRsnInformation);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagVendorSpecific);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagQbssLoadElement);

	tag = tag->next();
	num = tag->num_;
	EXPECT_EQ(num, GBeaconHdr::TagVendorSpecific);

	GBeaconHdr::TrafficIndicationMap* tiMap = GBeaconHdr::PTrafficIndicationMap(beaconHdr->getTag(GBeaconHdr::TagTrafficIndicationMap, sizeof(packet)));
	EXPECT_NE(tiMap, nullptr);
	EXPECT_EQ(tiMap->count_, 0);
	EXPECT_EQ(tiMap->period_, 1);
	EXPECT_EQ(tiMap->control_, 0);
	EXPECT_EQ(tiMap->bitmap_, 0);
}

#endif // GTEST



