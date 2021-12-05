#include "gtcphdr.h"

// ----------------------------------------------------------------------------
// GTcpHdr
// ----------------------------------------------------------------------------
//
// All tcpHdr field except tcpHdr.sum
// All data buffer(padding)
// ipHdr.ip_src, ipHdr.ip_dst, tcpHdrDataLen and IPPROTO_TCP
//
#ifndef Q_OS_ANDROID
uint16_t GTcpHdr::calcChecksum(GIpHdr* ipHdr, GTcpHdr* tcpHdr) {
#else
uint16_t GTcpHdr::calcChecksum(GIpHdr* ipHdrTemp, GTcpHdr* tcpHdr) {
	GVolatileIpHdr* ipHdr = PVolatileIpHdr(ipHdrTemp);
#endif
	uint32_t res = 0;
	int tcpHdrDataLen = ipHdr->len() - ipHdr->hl() * 4;

	// Add tcpHdr & data buffer as array of uint16_t
	uint16_t* p = reinterpret_cast<uint16_t*>(tcpHdr);
	for (int i = 0; i < tcpHdrDataLen / 2; i++) {
		res += htons(*p);
		p++;
	}

	// If length is odd, add last data(padding)
	if (tcpHdrDataLen % 2 != 0) {
		res += uint32_t(*(reinterpret_cast<uint8_t*>(p)) << 8);
	}

	// Decrease checksum from sum
	res -= tcpHdr->sum();

	// Add src address
#ifndef Q_OS_ANDROID
	uint32_t src = ipHdr->sip();
#else
	volatile uint32_t src = ipHdr->sip();
#endif
	res += ((src & 0xFFFF0000) >> 16) + (src & 0x0000FFFF);

	// Add dst address
#ifndef Q_OS_ANDROID
	uint32_t dst = ipHdr->dip();
#else
	volatile uint32_t dst = ipHdr->dip();
#endif
	res += ((dst & 0xFFFF0000) >> 16) + (dst & 0x0000FFFF);

	// Add extra information
	res += uint32_t(tcpHdrDataLen) + IPPROTO_TCP;

	// Recalculate sum
	if (res >> 16) {
		res = (res & 0xFFFF) + (res >> 16);
	}
	res = ~res;

	return uint16_t(res);
}

GBuf GTcpHdr::parseData(GIpHdr* ipHdr, GTcpHdr* tcpHdr) {
	GBuf res;
	res.size_ = ipHdr->len() - ipHdr->hl() * 4 - tcpHdr->off() * 4;
	if (res.size_ > 0)
		res.data_ = reinterpret_cast<u_char*>(tcpHdr) + tcpHdr->off() * 4;
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
struct GTcpHdrTest : testing::Test {
	GSyncPcapFile pcapFile_;
	void SetUp() override {
		pcapFile_.fileName_ = "pcap/eth-tcp-syn-port80.pcap";
		ASSERT_TRUE(pcapFile_.open());
	}
	void TearDown() override {
		ASSERT_TRUE(pcapFile_.close());
	}
};

TEST_F(GTcpHdrTest, allTest) {
	while (true) {
		GEthPacket packet;
		GPacket::Result res = pcapFile_.read(&packet);
		if (res != GPacket::Ok) break;
		packet.parse();

		GIpHdr* ipHdr = packet.ipHdr_;
		EXPECT_NE(ipHdr, nullptr);

		GTcpHdr* tcpHdr = packet.tcpHdr_;
		EXPECT_NE(tcpHdr, nullptr);

		//
		// field test
		//
		uint16_t dport = tcpHdr->dport();
		EXPECT_EQ(dport, 80);
		uint8_t off = tcpHdr->off();
		EXPECT_EQ(off, 8);
		uint8_t flags = tcpHdr->flags();
		uint8_t i = GTcpHdr::Syn;
		EXPECT_EQ(flags, i);

		//
		// checksum test
		//
		uint16_t realSum = tcpHdr->sum();
		uint16_t calcSum = GTcpHdr::calcChecksum(ipHdr, tcpHdr);
		EXPECT_EQ(realSum, calcSum);

		//
		// data test
		//
		GBuf data = GTcpHdr::parseData(ipHdr, tcpHdr);
		EXPECT_EQ(data.data_, nullptr);
		EXPECT_EQ(data.size_, 0);
	}
}

#endif // GTEST
