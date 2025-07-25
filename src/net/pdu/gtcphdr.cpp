#include "gtcphdr.h"

// ----------------------------------------------------------------------------
// GTcpHdr
// ----------------------------------------------------------------------------
//
// All tcpHdr field except tcpHdr.sum
// All data buffer(padding)
// ipHdr.sip, ipHdr.dip, tcpHdrDataLen and IPPROTO_TCP
//
uint16_t GTcpHdr::calcSum(GIpHdr* ipHdr, GTcpHdr* tcpHdr) { // Should disable compile optimization for GIPHdr(sip_ and dip_)
	qWarning() << "should change to inetCalcSum"; // by gilgil 2025.06.07

	uint32_t res = 0;
	int tcpHdrDataLen = ipHdr->tlen() - ipHdr->hlen() * 4;
	int loopCount = tcpHdrDataLen / 2;

	// Add tcpHdr & data buffer as array of uint16_t
	uint16_t* p = puint16_t(tcpHdr);
	for (int i = 0; i < loopCount; i++) {
		res += htons(*p);
		p++;
	}

	// If length is odd, add last data(padding)
	if (tcpHdrDataLen % 2 != 0)
		res += uint16_t(*puint8_t(p) << 8);

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

	// Add overflow value to the lower 16
	res = (res >> 16) + (res & 0xFFFF);
	res = ~res;

	return uint16_t(res);
}

uint16_t GTcpHdr::inetCalcSum(GIpHdr* ipHdr, GTcpHdr* tcpHdr) {
	uint32_t res = 0;
	int tcpHdrDataLen = ipHdr->tlen() - ipHdr->hlen() * 4;
	int loopCount = tcpHdrDataLen / 2;

	uint16_t* p = puint16_t(tcpHdr);
	for (int i = 0; i < loopCount; i++) {
		res += *p;
		p++;
	}

	if (tcpHdrDataLen % 2 != 0)
		res += uint16_t(*puint8_t(p));

	res -= tcpHdr->sum_;

#ifndef Q_OS_ANDROID
	uint32_t src = ipHdr->sip_;
#else
	volatile uint32_t src = ipHdr->sip_;
#endif
	res += ((src & 0xFFFF0000) >> 16) + (src & 0x0000FFFF);

	// Add dst address
#ifndef Q_OS_ANDROID
	uint32_t dst = ipHdr->dip_;
#else
	volatile uint32_t dst = ipHdr->dip_;
#endif
	res += ((dst & 0xFFFF0000) >> 16) + (dst & 0x0000FFFF);

	res += htons(uint32_t(tcpHdrDataLen)) + htons(IPPROTO_TCP);

	res = (res >> 16) + (res & 0xFFFF);
	res = ~res;

	return uint16_t(res);
}


GBuf GTcpHdr::parseData(GIpHdr* ipHdr, GTcpHdr* tcpHdr) {
	GBuf res;
	res.size_ = ipHdr->tlen() - ipHdr->hlen() * 4 - tcpHdr->off() * 4;
	if (res.size_ > 0)
		res.data_ = puchar(tcpHdr) + tcpHdr->off() * 4;
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
TEST(GTcpHdrTest, synFileTest) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/test/eth-tcp-syn-port80.pcap";
	ASSERT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Eth);

	while (true) {
		GEthPacket packet;
		GPacket::Result res = pcapFile.read(&packet);
		if (res != GPacket::Ok) break;

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
		// calcSum test
		//
		uint16_t realSum = tcpHdr->sum();
		uint16_t calcSum = GTcpHdr::calcSum(ipHdr, tcpHdr);
		EXPECT_EQ(realSum, calcSum);

		//
		// inetCalcSum test
		//
		uint16_t inetCalcSum = GTcpHdr::inetCalcSum(ipHdr, tcpHdr);
		EXPECT_EQ(realSum, ntohs(inetCalcSum));

		//
		// recalcSum(change dip - 32 bit) test
		//
		uint32_t backupInetDip = ipHdr->dip_;
		{
			uint16_t oldSum = tcpHdr->sum();
			uint32_t oldValue = ipHdr->dip();
			ipHdr->dip_ = htonl(GIp("8.8.8.9"));
			uint32_t newValue = ipHdr->dip();

			uint16_t newSum = GTcpHdr::calcSum(ipHdr, tcpHdr);
			uint16_t recalcSum = GIpHdr::recalcSum(oldSum, oldValue, newValue);
			EXPECT_EQ(newSum, recalcSum);

			uint16_t inetNewSum = GTcpHdr::inetCalcSum(ipHdr, tcpHdr);
			EXPECT_EQ(ntohs(inetNewSum), recalcSum);
		}
		ipHdr->dip_ = backupInetDip;

		//
		// data test
		//
		GBuf data = GTcpHdr::parseData(ipHdr, tcpHdr);
		EXPECT_EQ(data.data_, nullptr);
		EXPECT_EQ(data.size_, 0);
	}

	EXPECT_TRUE(pcapFile.close());
}

TEST(GTcpHdrTest, checksumTest) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/test/eth-tcp-port4660-abcd.pcap";
	ASSERT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Eth);

	uint16_t realIpSums[] = { 0xC7F9, 0x24B6, 0xC800, 0xC7FE, 0x2A18, 0x2A16, 0xC7FE, 0xC7FB, 0x2A14, 0xC7FC, 0xC7F8, 0x2A12, 0xC7FA, 0xC7F5, 0x2A10, 0xC7F8, 0xC7F7, 0x2A13, 0xC7F6 };
	uint16_t realTcpSums[] = { 0x6131, 0x758A, 0xA079, 0x5B54, 0x97ED, 0x56E3, 0x97A4, 0x4DA9, 0x44D4, 0x860D, 0xFAC5, 0xF386, 0x77A0, 0xEC62, 0xE583, 0x69EE, 0x629F, 0x5B04, 0x5ACC };

	for (int i = 0; i < 19; i++) {
		GEthPacket packet;
		GPacket::Result res = pcapFile.read(&packet);
		if (res != GPacket::Ok) break;

		GIpHdr* ipHdr = packet.ipHdr_;
		EXPECT_NE(ipHdr, nullptr);

		uint16_t realIpSum = realIpSums[i];
		uint16_t calcIpSum = GIpHdr::calcSum(ipHdr);
		EXPECT_EQ(realIpSum, calcIpSum);

		uint16_t inetCalcIpSum = GIpHdr::inetCalcSum(ipHdr);
		EXPECT_EQ(realIpSum, ntohs(inetCalcIpSum));

		GTcpHdr* tcpHdr = packet.tcpHdr_;
		EXPECT_NE(tcpHdr, nullptr);

		uint16_t realTcpSum = realTcpSums[i];
		uint16_t calcTcpSum = GTcpHdr::calcSum(ipHdr, tcpHdr);
		EXPECT_EQ(realTcpSum, calcTcpSum);

		uint16_t inetCalcTcpSum = GTcpHdr::inetCalcSum(ipHdr, tcpHdr);
		EXPECT_EQ(realTcpSum, htons(inetCalcTcpSum));
	}

	EXPECT_TRUE(pcapFile.close());
}

#endif // GTEST
