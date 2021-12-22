#include "cppcapfile.h"

bool CPcapFile::doOpen() {
	return CPcap::openFile(fileName_, filter_);
}

bool CPcapFile::doClose() {
	return CPcap::doClose();
}

#ifdef GTEST
#include <gtest/gtest.h>

TEST(PcapFile, openCloseTest) {
	CPcapFile file;
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
