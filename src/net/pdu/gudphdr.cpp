#include "gudphdr.h"

// ----------------------------------------------------------------------------
// GUdpHdr
// ----------------------------------------------------------------------------
//
// All udpHdr field except udpHdr.uh_sum
// All data buffer(padding)
// ipHdr.ip_src, ipHdr.ip_dst, udpHdrDataLen and IPPROTO_UDP
//
uint16_t GUdpHdr::calcChecksum(GIpHdr* ipHdr, GUdpHdr* udpHdr) {
	uint32_t res = 0;
	int udpHdrDataLen = udpHdr->len();

	// Add udpHdr & data buffer as array of uint16_t
	uint16_t* p = puint16_t(udpHdr);
	for (int i = 0; i < udpHdrDataLen / 2; i++) {
		res += htons(*p);
		p++;
	}

	// If length is odd, add last data(padding)
	if (udpHdrDataLen % 2 != 0)
		res += uint32_t(*puint8_t(p) << 8);

	// Decrease checksum from sum
	res -= udpHdr->sum();

	// Add src address
	uint32_t src = ipHdr->sip();
	res += ((src & 0xFFFF0000) >> 16) + (src & 0x0000FFFF);

	// Add dst address
	uint32_t dst = ipHdr->dip();
	res += ((dst & 0xFFFF0000) >> 16) + (dst & 0x0000FFFF);

	// Add extra information
	res += uint32_t(udpHdrDataLen) + IPPROTO_UDP;

	// Recalculate sum
	if (res >> 16)
		res = (res & 0xFFFF) + (res >> 16);
	res = ~res;

	return uint16_t(res);
}

GBuf GUdpHdr::parseData(GUdpHdr* udpHdr) {
	GBuf res;
	res.size_ = udpHdr->len() - sizeof(GUdpHdr);
	if (res.size_ > 0)
		res.data_ = puchar(udpHdr) + sizeof(GUdpHdr);
	else
		res.data_ = nullptr;
	return res;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

#include "net/capture/gsyncpcapfile.h"
TEST(GUdpHdrTest, ipv4FileTest) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/test/ipv4-udp-port1234.pcap";
	ASSERT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Ip);

	while (true) {
		GIpPacket packet;
		GPacket::Result res = pcapFile.read(&packet);
		if (res != GPacket::Ok) break;

		GIpHdr* ipHdr = packet.ipHdr_;
		EXPECT_NE(ipHdr, nullptr);

		GUdpHdr* udpHdr = packet.udpHdr_;
		EXPECT_NE(udpHdr, nullptr);

		//
		// field test
		//
		uint16_t dport = udpHdr->dport();
		EXPECT_EQ(dport, 1234);
		uint16_t len = udpHdr->len();
		EXPECT_EQ(len, 12);

		//
		// checksum test
		//
		uint16_t realSum = udpHdr->sum();
		uint16_t calcSum = GUdpHdr::calcChecksum(ipHdr, udpHdr);
		EXPECT_EQ(realSum, calcSum);

		//
		// data test
		//
		GBuf data = GUdpHdr::parseData(udpHdr);
		EXPECT_NE(data.data_, nullptr);
		EXPECT_EQ(data.size_, 4);
	}

	EXPECT_TRUE(pcapFile.close());
}

TEST(GUdpHdrTest, checksumTest) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/test/eth-udp-port4660-abcd.pcap";
	ASSERT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Eth);

	uint16_t realIpSums[] = { 0x965E, 0xD0B5, 0x965C, 0xCE99, 0x965A, 0xCE11, 0x9658, 0xCCAC };
	uint16_t realUdpSums[] = { 0x846E, 0x846E, 0x842A, 0x842A, 0x4128, 0x4128, 0x40E2, 0x40E2 };

	for (int i = 0; i < 8; i++) {
		GEthPacket packet;
		GPacket::Result res = pcapFile.read(&packet);
		if (res != GPacket::Ok) break;

		GIpHdr* ipHdr = packet.ipHdr_;
		EXPECT_NE(ipHdr, nullptr);
		uint16_t realIpSum = realIpSums[i];
		uint16_t calcIpSum = GIpHdr::calcChecksum(ipHdr);
		EXPECT_EQ(realIpSum, calcIpSum);

		GUdpHdr* udpHdr = packet.udpHdr_;
		EXPECT_NE(udpHdr, nullptr);
		uint16_t realUdpSum = realUdpSums[i];
		uint16_t calcUdpSum = GUdpHdr::calcChecksum(ipHdr, udpHdr);
		EXPECT_EQ(realUdpSum, calcUdpSum);
	}

	EXPECT_TRUE(pcapFile.close());
}

#endif // GTEST
