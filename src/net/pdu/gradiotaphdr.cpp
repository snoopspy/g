#include "gradiotaphdr.h"

struct AlignSizeInfo {
	int size_;
	int align_;
} _alignSizeInfo[] {
	{8, 8}, // 0, Tsft (u64 mactime)
	{1, 1}, // 1, Flags (u8 flags)
	{1, 1}, // 2, Rate (u8)
	{4, 2}, // 3, Channel (u16 frequency, u16 flags)
	{2, 2}, // 4, Fhss (u8 hop set, u8 hop pattern)
	{1, 1}, // 5, AntennaSignal (s8)
	{1, 1}, // 6, AntennaNoise (s8)
	{2, 2}, // 7, LockQuality (u16)
	{1, 1}, // 8, TxAttenuation *
	{1, 1}, // 9, DbTxAttenuation *
	{1, 1}, // 10, DbmTxPower *
	{1, 1}, // 11, Antenna *
	{1, 1}, // 12, DbAntennaSignal *
	{1, 1}, // 13, DbAntennaNoise *
	{2, 2}, // 14, RxFlags (u16)
	{1, 1}, // 15, TxFlags *
	{1, 1}, // 16, RtsRetries *
	{1, 1}, // 17, DataRetries *
	{1, 1}, // 18, XChannel *
	{1, 1}, // 19, Mcs *
	{1, 1}, // 20, AMpdu *
	{1, 1}, // 21, Vht *
	{12, 8}, // 22, Times (u64 timestamp, u16 accuracy, u8 unit/position, u8 flags)
	{1, 1}, // 23, He *
	{1, 1}, // 24, HeMu *
	{1, 1}, // 25, HeMuOt *herUser
	{1, 1}, // 26, ZeroLeng *hPsdu
	{1, 1}, // 27, LSig *
	{1, 1}, // 28, Tlv *
	{0, 0}, // 29, Radio *tapNamespace
	{1, 1}, // 30, VendorN *amespace
	{0, 0}, // 31, Ext *
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

QList<QByteArray> GRadiotapHdr::getInfo(BitNo bitNo) {
	QList<QByteArray> res;

	char* value = pchar(this);
	int offset = sizeof(GRadiotapHdr);

	Present* present = &present_;
	int presentFlagCount = 1;
	while ((present ->p_ & 0x80000000) != 0) {
		presentFlagCount++;
		offset += sizeof(Present);
		present = present->next();
	}

	present = &present_;
	for (int pf = 0; pf < presentFlagCount; pf++) {
		le32_t mask = 1;
		for (le32_t i = 0; i < 31; i++) {
			if ((present->p_ & mask) != 0) {
				int align = _alignSizeInfo[i].align_;
				if (align > 1)
					offset = (offset + align - 1) / align * align;

				int size = _alignSizeInfo[i].size_;
				if (i == bitNo) {
					char* p = value + offset;
					QByteArray ba(p, size);
					res.push_back(ba);
				}
				offset += size;
			}
			mask <<= 1;
		}
		present = present->next();
	}

	return res;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

QVector<QString> pcapFileNames {
	"pcap/dot11/beacon-a2000ua-testap.pcap",
	"pcap/dot11/beacon-awus051nh-testap5g.pcap",
	"pcap/dot11/beacon-awus051nh-testap.pcap",
	"pcap/dot11/beacon-galaxy7-testap5g.pcap",
	"pcap/dot11/beacon-galaxy7-testap.pcap",
	"pcap/dot11/beacon-nexus5-testap5g.pcap",
	"pcap/dot11/beacon-nexus5-testap.pcap",
};

#include <GSyncPcapFile>
TEST(RadiotapHdr, beacon_a2000ua_testap) {

	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/dot11/beacon-a2000ua-testap.pcap";
	EXPECT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Dot11);

	GDot11Packet packet;
	EXPECT_EQ(pcapFile.read(&packet), GPacket::Ok);

	GRadiotapHdr* radiotapHdr = packet.radiotapHdr_;
	EXPECT_NE(radiotapHdr, nullptr);
	EXPECT_EQ(radiotapHdr->len_, 32);
	EXPECT_EQ(le32_t(radiotapHdr->present_), 0xa00040ae);

	QList<QByteArray> ba = radiotapHdr->getInfo(GRadiotapHdr::Flags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0], QByteArray("\x10"));

	ba = radiotapHdr->getInfo(GRadiotapHdr::Rate);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0], QByteArray("\x02")); // 500Kbps * 2 == 1.0 Mb/s

	ba = radiotapHdr->getInfo(GRadiotapHdr::Channel);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0], QByteArray("\x8a\x09\xa0\x00", 4)); // 2442, 0x00a0

	ba = radiotapHdr->getInfo(GRadiotapHdr::AntennaSignal);
	EXPECT_EQ(ba.count(), 3);
	EXPECT_EQ(ba[0], QByteArray("\xb4")); // -76
	EXPECT_EQ(ba[1], QByteArray("\xae")); // -82
	EXPECT_EQ(ba[2], QByteArray("\xae")); // -82


	EXPECT_TRUE(pcapFile.close());
}

#endif // GTEST
