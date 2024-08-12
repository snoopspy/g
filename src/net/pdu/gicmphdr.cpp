#include "gicmphdr.h"

// ----------------------------------------------------------------------------
// GIcmpHdr
// ----------------------------------------------------------------------------
uint16_t GIcmpHdr::calcChecksum(GIpHdr* ipHdr, GIcmpHdr* icmpHdr) {
	uint32_t res = 0;
	int icmpHdrDataLen = ipHdr->tlen() - ipHdr->hlen() * 4;

	// Add udpHdr & data buffer as array of uint16_t
	uint16_t* p = reinterpret_cast<uint16_t*>(icmpHdr);
	for (int i = 0; i < icmpHdrDataLen / 2; i++) {
		res += htons(*p);
		p++;
	}

	// If length is odd, add last data(padding)
	if (icmpHdrDataLen % 2 != 0)
		res += uint32_t(*(reinterpret_cast<uint8_t*>(p)) << 8);

	// Decrease checksum from sum
	res -= icmpHdr->sum();

	// Recalculate sum
	if (res >> 16)
		res = (res & 0xFFFF) + (res >> 16);
	res = ~res;

	return uint16_t(res);
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

#include "net/capture/gsyncpcapfile.h"
struct GIcmpHdrTest : testing::Test {
	GSyncPcapFile pcapFile_;
	void SetUp() override {
		pcapFile_.fileName_ = "pcap/test/eth-icmp8.pcap";
		ASSERT_TRUE(pcapFile_.open());
	}
	void TearDown() override {
		ASSERT_TRUE(pcapFile_.close());
	}
};

TEST_F(GIcmpHdrTest, allTest) {
	int frameNumber = 1;
	while (true) {
		GEthPacket packet;
		GPacket::Result res = pcapFile_.read(&packet);
		if (res != GPacket::Ok) break;

		GIpHdr* ipHdr = packet.ipHdr_;
		EXPECT_NE(ipHdr, nullptr);

		GIcmpHdr* icmpHdr = packet.icmpHdr_;
		EXPECT_NE(icmpHdr, nullptr);

		//
		// field test
		//
		if (frameNumber++ % 2 == 1)
			EXPECT_EQ(icmpHdr->type(), GIcmpHdr::PingRequest);
		else
			EXPECT_EQ(icmpHdr->type(), GIcmpHdr::PingReply);
		EXPECT_EQ(icmpHdr->code(), 0);

		//
		// checksum test
		//
		uint16_t realSum = icmpHdr->sum();
		uint16_t calcSum = GIcmpHdr::calcChecksum(ipHdr, icmpHdr);
		EXPECT_EQ(realSum, calcSum);
	}
}

#endif // GTEST
