#include "gdot11hdr.h"

GDot11Hdr* GDot11Hdr::check(GRadiotapHdr* radiotapHdr, uint32_t size) {
	uint32_t len = radiotapHdr->len_;
	if (size < sizeof(GDot11Hdr) + len) {
		qWarning() << QString("invalid size radiotapHdr->len=%1 size=%2").arg(len).arg(size);
		return nullptr;
	}
	GDot11Hdr* dot11Hdr = PDot11Hdr(pchar(radiotapHdr) + len);
	return dot11Hdr;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(Dot11Hdr, typeTest) {
	uint8_t packet[] = { 0xd4, 0x10 }; // dot11-sample.pcap frame.number==10
	GDot11Hdr* dot11Hdr = PDot11Hdr(packet);

	le8_t ver = dot11Hdr->ver_;
	EXPECT_EQ(ver, 0x00);

	le8_t type = dot11Hdr->type_;
	EXPECT_EQ(type, GDot11Hdr::ControlFrame);

	le8_t subtype = dot11Hdr->subtype_;
	EXPECT_EQ(subtype, 0x0D);

	le8_t typeSubtype = dot11Hdr->typeSubtype();
	EXPECT_EQ(typeSubtype, GDot11Hdr::Ack);
}

#endif // GTEST
