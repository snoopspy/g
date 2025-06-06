#include "gicmphdr.h"

// ----------------------------------------------------------------------------
// GIcmpHdr
// ----------------------------------------------------------------------------
uint16_t GIcmpHdr::calcChecksum(GIpHdr* ipHdr, GIcmpHdr* icmpHdr) {
	uint32_t res = 0;
	int icmpHdrDataLen = ipHdr->tlen() - ipHdr->hlen() * 4;
	int loopCount = icmpHdrDataLen / 2;

	// Add udpHdr & data buffer as array of uint16_t
	uint16_t* p = puint16_t(icmpHdr);
	for (int i = 0; i < loopCount; i++) {
		res += htons(*p);
		p++;
	}

	// If length is odd, add last data(padding)
	if (icmpHdrDataLen % 2 != 0)
		res += uint16_t(*puint8_t(p) << 8);

	// Decrease checksum from sum
	res -= icmpHdr->sum();

	// Add overflow value to the lower 16
	res = (res & 0xFFFF) + (res >> 16);
	res = ~res;

	return uint16_t(res);
}

uint16_t GIcmpHdr::inetCalcChecksum(GIpHdr* ipHdr, GIcmpHdr* icmpHdr) {
	uint32_t res = 0;
	int icmpHdrDataLen = ipHdr->tlen() - ipHdr->hlen() * 4;
	int loopCount = icmpHdrDataLen / 2;

	uint16_t* p = puint16_t(icmpHdr);
	for (int i = 0; i < loopCount; i++) {
		res += *p;
		p++;
	}

	if (icmpHdrDataLen % 2 != 0)
		res += uint16_t(*puint8_t(p));

	res -= icmpHdr->sum_;

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
TEST(GIcmpHdrTest, icmp8Test) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/test/eth-icmp8.pcap";
	ASSERT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Eth);

	int frameNumber = 1;
	while (true) {
		GEthPacket packet;
		GPacket::Result res = pcapFile.read(&packet);
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

		uint16_t inetCalcSum = GIcmpHdr::inetCalcChecksum(ipHdr, icmpHdr);
		EXPECT_EQ(realSum, ntohs(inetCalcSum));
	}
}

TEST(GIcmpHdrTest, checksumTest) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/test/eth-icmp8.pcap";
	ASSERT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Eth);

	uint16_t realIpSums[] = { 0xF5D4, 0x2BBB, 0xF547, 0x2BBB, 0xF4D1, 0x2BBB, 0xF4CF, 0x2BBB };
	uint16_t realIcmpSums[] = { 0x347F, 0x3C7F, 0xEA77, 0xF277, 0xDA70, 0xE270, 0x7D6A, 0x856A };

	for (int i = 0; i < 8; i++) {
		GEthPacket packet;
		GPacket::Result res = pcapFile.read(&packet);
		if (res != GPacket::Ok) break;

		GIpHdr* ipHdr = packet.ipHdr_;
		EXPECT_NE(ipHdr, nullptr);

		uint16_t realIpSum = realIpSums[i];
		uint16_t calcIpSum = GIpHdr::calcChecksum(ipHdr);
		EXPECT_EQ(realIpSum, calcIpSum);

		uint16_t inetCalcIpSum = GIpHdr::inetCalcChecksum(ipHdr);
		EXPECT_EQ(realIpSum, ntohs(inetCalcIpSum));

		GIcmpHdr* icmpHdr = packet.icmpHdr_;
		EXPECT_NE(icmpHdr, nullptr);

		uint16_t realIcmpSum = realIcmpSums[i];
		uint16_t calcUdpSum = GIcmpHdr::calcChecksum(ipHdr, icmpHdr);
		EXPECT_EQ(realIcmpSum, calcUdpSum);

		uint16_t inetCalcIcmpSum = GIcmpHdr::inetCalcChecksum(ipHdr, icmpHdr);
		EXPECT_EQ(realIcmpSum, htons(inetCalcIcmpSum));
	}

	EXPECT_TRUE(pcapFile.close());
}
#endif // GTEST
