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
	uint16_t* p = reinterpret_cast<uint16_t*>(udpHdr);
	for (int i = 0; i < udpHdrDataLen / 2; i++) {
		res += htons(*p);
		p++;
	}

	// If length is odd, add last data(padding)
	if (udpHdrDataLen % 2 != 0)
		res += uint32_t(*(reinterpret_cast<uint8_t*>(p)) << 8);

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
	while (res >> 16) {
		res = (res & 0xFFFF) + (res >> 16);
	}
	res = ~res;

	return uint16_t(res);
}

GBuf GUdpHdr::parseData(GUdpHdr* udpHdr) {
	GBuf res;
	res.size_ = udpHdr->len() - sizeof(GUdpHdr);
	if (res.size_ > 0)
		res.data_ = reinterpret_cast<u_char*>(udpHdr) + sizeof(GUdpHdr);
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
#include "net/packet/gippacket.h"
struct GUdpHdrTest : testing::Test {
	GSyncPcapFile pcapFile_;
	void SetUp() override {
		pcapFile_.fileName_ = "pcap/ipv4-udp-port1234.pcap";
		ASSERT_TRUE(pcapFile_.open());
	}
	void TearDown() override {
		ASSERT_TRUE(pcapFile_.close());
	}
};

TEST_F(GUdpHdrTest, allTest) {
	while (true) {
		GIpPacket packet;
		GPacket::Result res = pcapFile_.read(&packet);
		if (res != GPacket::Ok) break;
		packet.parse();

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
}

#endif // GTEST
