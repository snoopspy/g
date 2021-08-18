#include "gdot11hdr.h"

Dot11Hdr* Dot11Hdr::check(RadiotapHdr* radiotapHdr, uint32_t size) {
	uint32_t len = radiotapHdr->len_;
	Dot11Hdr* dot11Hdr = PDot11Hdr(pchar(radiotapHdr) + len);
	if (size < sizeof(Dot11Hdr) + len) {
		//GTRACE("invalid size radiotapHdr->len=%u size=%u", len, size);
		//dump(puchar(radiotapHdr), len /*size*/); // gilgil temp
		return nullptr;
	}
	return dot11Hdr;
}

#ifdef GTEST
#include <gtest/gtest.h>

TEST(Dot11Hdr, typeTest) {
	uint8_t packet[] = { 0xd4, 0x10 }; // dot11-sample.pcap frame.number==10
	Dot11Hdr* dot11Hdr = PDot11Hdr(packet);

	le8_t ver = dot11Hdr->ver_;
	EXPECT_EQ(ver, 0x00);

	le8_t type = dot11Hdr->type_;
	EXPECT_EQ(type, Dot11Hdr::Control);

	le8_t subtype = dot11Hdr->subtype_;
	EXPECT_EQ(subtype, 0x0D);

	le8_t typeSubtype = dot11Hdr->typeSubtype();
	EXPECT_EQ(typeSubtype, Dot11Hdr::Acknowledgement);
}

#endif // GTEST
