#include "giphdr.h"

// ----------------------------------------------------------------------------
// GIpHdr
// ----------------------------------------------------------------------------
//
// All ipHdr field except ipHdr.sum
//
uint16_t GIpHdr::calcChecksum(GIpHdr* ipHdr) {
	uint32_t res = 0;
	uint16_t *p;

	// Add ipHdr buffer as array of uint16_t
	p = puint16_t(ipHdr);
	for (int i = 0; i < int(sizeof(GIpHdr)) / 2; i++) {
		res += ntohs(*p);
		p++;
	}

	// Do not consider padding because ip header length is always multilpe of 2.

	// Decrease checksum from sum
	res -= ipHdr->sum();

	// Recalculate sum
	res = (res & 0xFFFF) + (res >> 16);
	res = ~res;

	return uint16_t(res);
}

uint16_t GIpHdr::recalcChecksum(uint16_t oldChecksum, uint16_t oldValue, uint16_t newValue) {
	uint32_t res = oldValue + (~newValue & 0xFFFF);
	res += oldChecksum;
	res = (res & 0xFFFF) + (res >> 16);
	return uint16_t(res + (res >> 16));
}

uint16_t GIpHdr::recalcChecksum(uint16_t oldChecksum, uint32_t oldValue, uint32_t newValue) {
	uint16_t oldValue16;
	uint16_t newValue16;
	uint16_t res;

	oldValue16 = (oldValue & 0xFFFF0000) >> 16;
	newValue16 = (newValue & 0xFFFF0000) >> 16;
	res = recalcChecksum(oldChecksum, oldValue16, newValue16);

	oldValue16 = oldValue & 0x0000FFFF;
	newValue16 = newValue & 0x0000FFFF;
	res = recalcChecksum(res, oldValue16, newValue16);

	return res;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

#include "net/capture/gsyncpcapfile.h"
TEST(GIpHdrTest, synFileTest) {
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

		//
		// field test
		//
		EXPECT_EQ(ipHdr->v(), 4);
		EXPECT_EQ(ipHdr->hlen(), 5);
		EXPECT_EQ(ipHdr->sip(), GIp("10.1.1.48"));
		EXPECT_EQ(ipHdr->dip(), GIp("8.8.8.8"));

		{
			//
			// calcChecksum test
			//
			GIpHdr ipHdr = *packet.ipHdr_;
			uint16_t realSum = ipHdr.sum();
			uint16_t calcSum = GIpHdr::calcChecksum(&ipHdr);
			EXPECT_EQ(realSum, calcSum);
		}
		{
			//
			// recalcChecksum(decrease ttl - 16 bit) test
			//
			GIpHdr ipHdr = *packet.ipHdr_;
			uint16_t oldChecksum = ipHdr.sum();
			uint16_t oldValue = uint16_t(ipHdr.ttl() << 8) + ipHdr.p();
			ipHdr.ttl_--;
			uint16_t newValue = uint16_t(ipHdr.ttl() << 8) + ipHdr.p();
			uint16_t newChecksum = GIpHdr::calcChecksum(&ipHdr);
			uint16_t recalcChecksum = GIpHdr::recalcChecksum(oldChecksum, oldValue, newValue);
			EXPECT_EQ(newChecksum, recalcChecksum);
		}
		{
			//
			// recalcChecksum(change dip - 32 bit) test
			//
			GIpHdr ipHdr = *packet.ipHdr_;
			uint16_t oldChecksum = ipHdr.sum();
			uint32_t oldValue = ipHdr.dip();
			ipHdr.dip_ = GIp("1.2.3.4");
			uint32_t newValue = ipHdr.dip();
			uint16_t newChecksum = GIpHdr::calcChecksum(&ipHdr);
			uint16_t recalcChecksum = GIpHdr::recalcChecksum(oldChecksum, oldValue, newValue);
			EXPECT_EQ(newChecksum, recalcChecksum);
		}
	}

	EXPECT_TRUE(pcapFile.close());
}

#endif // GTEST
