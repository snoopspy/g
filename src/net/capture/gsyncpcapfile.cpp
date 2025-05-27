#include "gsyncpcapfile.h"

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(GSyncPcapFile, icmpFileTest) {
	GSyncPcapFile file;
	file.fileName_ = "pcap/test/eth-icmp8.pcap";
	ASSERT_TRUE(file.open());

	GEthPacket packet;
	for (int i = 0; i < 8; i++) {
		GPacket::Result res = file.read(&packet);
		ASSERT_TRUE(res == GPacket::Ok);
	}
	GPacket::Result res = file.read(&packet);
	ASSERT_TRUE(res == GPacket::Eof);

	ASSERT_TRUE(file.close());
}

#endif // GTEST
