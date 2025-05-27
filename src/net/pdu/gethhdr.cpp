#include "gethhdr.h"

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

#include "net/capture/gsyncpcapfile.h"
TEST(GEthHdrTest, synFileTest) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/test/eth-tcp-syn-port80.pcap";
	ASSERT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Eth);

	while (true) {
		GEthPacket packet;
		GPacket::Result res = pcapFile.read(&packet);
		if (res != GPacket::Ok) break;

		GEthHdr* ethHdr = packet.ethHdr_;
		EXPECT_NE(ethHdr, nullptr);

		//
		// field test
		//
		GMac mac1, mac2;
		mac1 = ethHdr->smac();
		mac2 = GMac("ac:fd:ce:47:83:d0");
		EXPECT_EQ(mac1, mac2);

		mac1 = ethHdr->dmac();
		mac2 = GMac("90:9f:33:d9:a0:e0");
		EXPECT_EQ(mac1, mac2);

		uint16_t type = ethHdr->type();
		EXPECT_EQ(type, GEthHdr::Ip4);
	}

	EXPECT_TRUE(pcapFile.close());
}

#endif // GTEST
