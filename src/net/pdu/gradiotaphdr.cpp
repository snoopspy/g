#include "gradiotaphdr.h"

struct AlignSizeInfo {
	int align_;
	int size_;
} alignSizeInfo_[2] {
	{8, 8}, // Tsft
	{1, 1}
};

GRadiotapHdr* GRadiotapHdr::check(gbyte* p, uint32_t size) {
	GRadiotapHdr* radiotapHdr = PRadiotapHdr(p);
	le16_t len = radiotapHdr->len_;
	if (len < sizeof(GRadiotapHdr) || len > size) {
		qWarning() << QString("len=%1 size=%2").arg(len).arg(size);
		return nullptr;
	}
	return radiotapHdr;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

QVector<QString> pcapFileNames {
	"pcap/dot11/beacon-a2000ua-testap.pcap",
	"pcap/dot11/beacon-a2000ua-testap.pcap",
	"pcap/dot11/beacon-a2000ua-testap.pcap",
	"pcap/dot11/beacon-a2000ua-testap.pcap"
};

#include <GSyncPcapFile>
TEST(RadioTapHdrHdr, lengthCheck) {
	struct Info {
		int len_;
		le8_t rate;
		le16_t frequency_;
		le8_t signal_;
	};

	QVector<int> lengths {
		32,
		32,
		32,
		32
	};

	GSyncPcapFile pcapFile;
	for (int i = 0; i < pcapFileNames.count(); i++) {
		QString pcapFileName = pcapFileNames[i];
		pcapFile.fileName_ = pcapFileName;
		EXPECT_TRUE(pcapFile.open());
		EXPECT_EQ(pcapFile.dlt(), GPacket::Dot11);

		GDot11Packet packet;
		EXPECT_EQ(pcapFile.read(&packet), GPacket::Ok);
		EXPECT_EQ(packet.radiotapHdr_->len_, lengths[i]);
		EXPECT_TRUE(pcapFile.close());
	}
}

#endif // GTEST
