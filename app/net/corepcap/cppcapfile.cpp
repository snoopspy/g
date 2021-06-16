#include "cppcapfile.h"

bool LPcapFile::doOpen() {
	return LPcap::openFile(fileName_, filter_);
}

bool LPcapFile::doClose() {
	return LPcap::doClose();
}

#ifdef GTEST
#include <gtest/gtest.h>

TEST(PcapFile, openCloseTest) {
	LPcapFile file;
	EXPECT_FALSE(file.open());
	file.close();

	file.fileName_ = "do-not-exist.pcap";
	EXPECT_FALSE(file.open());
	file.close();

	file.fileName_ = "pcap/eth-test.gilgil.net.pcap";
	EXPECT_TRUE(file.open());
	file.close();
}

#endif // GTEST
