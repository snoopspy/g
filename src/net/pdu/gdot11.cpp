#include "gdot11.h"

// ----------------------------------------------------------------------------
// GDot11
// ----------------------------------------------------------------------------
GDot11* GDot11::check(GRadioHdr* radioHdr, uint32_t size) {
	uint32_t len = radioHdr->len_;
	if (size < sizeof(GDot11) + len) {
		qWarning() << QString("invalid size radioHdr->len=%1 size=%2").arg(len).arg(size);
		return nullptr;
	}
	GDot11* dot11Hdr = PDot11(pchar(radioHdr) + len);
	return dot11Hdr;
}

void GDot11::init(TypeSubtype typeSubtype) {
	ver_ = 0;
	type_ = typeSubtype & 0xC0;
	subtype_ = typeSubtype & 0x0F;
	flags_ = 0;
	duration_ = 0;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(Dot11Hdr, typeTest) {
	uint8_t packet[] = { 0xd4, 0x10 }; // dot11-sample.pcap frame.number==10
	GDot11* dot11Hdr = PDot11(packet);

	le8_t ver = dot11Hdr->ver_;
	EXPECT_EQ(ver, 0x00);

	le8_t type = dot11Hdr->type_;
	EXPECT_EQ(type, GDot11::ControlFrame);

	le8_t subtype = dot11Hdr->subtype_;
	EXPECT_EQ(subtype, 0x0D);

	le8_t typeSubtype = dot11Hdr->typeSubtype();
	EXPECT_EQ(typeSubtype, GDot11::Ack);
}

#endif // GTEST
