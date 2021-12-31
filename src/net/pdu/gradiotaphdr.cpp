#include "gradiotaphdr.h"

struct AlignSizeInfo {
	int size_;
	int align_;
} _alignSizeInfo[] {
	{8, 8}, // Tsft (u64 mactime)
	{1, 1}, // Flags (u8 flags)
	{1, 1}, // Rate (u8)
	{4, 2}, // Channel (u16 frequency, u16 flags)
	{2, 2}, // Fhss (u8 hop set, u8 hop pattern)
	{1, 1}, // AntennaSignal (s8)
	{1, 1}, // AntennaNoise (s8)
	{2, 2}, // LockQuality (u16)
	{1, 1}, // TxAttenuation *
	{1, 1}, // DbTxAttenuation *
	{1, 1}, // DbmTxPower *
	{1, 1}, // Antenna *
	{1, 1}, // DbAntennaSignal *
	{1, 1}, // DbAntennaNoise *
	{2, 2}, // RxFlags (u16)
	{1, 1}, // TxFlags *
	{1, 1}, // RtsRetries *
	{1, 1}, // DataRetries *
	{1, 1}, // XChannel *
	{1, 1}, // Mcs *
	{1, 1}, // AMpdu *
	{1, 1}, // Vht *
	{1, 1}, // Times *tamp
	{1, 1}, // He *
	{1, 1}, // HeMu *
	{1, 1}, // HeMuOt *herUser
	{1, 1}, // ZeroLeng *hPsdu
	{1, 1}, // LSig *
	{1, 1}, // Tlv *
	{0, 0}, // Radio *tapNamespace
	{1, 1}, // VendorN *amespace
	{1, 1},// Ext *
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

QList<QByteArray> GRadiotapHdr::Present::getInfo(BitNo bitNo) {
	QList<QByteArray> res;

	Present* present = this;
	int presentFlagCount = 1;
	while ((present ->p_ & 0x80000000) != 0) {
		presentFlagCount++;
		present = present->next();
	}

	char* presentValue = pchar(present + 1);
	present = this;
	for (int i = 0; i < presentFlagCount; i++) {
		le32_t mask = 1;
		int offset = 0;
		for (le32_t i = 0; i < bitNo; i++) {
			if ((present->p_ & mask) != 0) {
				int size = _alignSizeInfo[i].size_;
				int align = _alignSizeInfo[i].align_;
				if (align > 1)
					offset = (offset + align - 1) / align * align;
				offset += size;
			}
			mask <<= 1;
		}
		if ((present->p_ & mask) != 0) {
			char* p = presentValue + offset;
			QByteArray ba(p, _alignSizeInfo[bitNo].size_);
			res.push_back(ba);
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

	QList<QByteArray> ba = radiotapHdr->present_.getInfo(GRadiotapHdr::Present::Flags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0], QByteArray("\x10"));

	ba = radiotapHdr->present_.getInfo(GRadiotapHdr::Present::Rate);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0], QByteArray("\x02")); // 500Kbps * 2 == 1.0 Mb/s

	ba = radiotapHdr->present_.getInfo(GRadiotapHdr::Present::Channel);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0], QByteArray("\x8a\x09\xa0\x00", 4)); // 2442, 0x00a0

	ba = radiotapHdr->present_.getInfo(GRadiotapHdr::Present::AntennaSignal);
	EXPECT_EQ(ba.count(), 3);
	EXPECT_EQ(ba[0], QByteArray("\xb4")); // -76
	EXPECT_EQ(ba[1], QByteArray("\xae")); // -82
	EXPECT_EQ(ba[2], QByteArray("\xae")); // -82


	EXPECT_TRUE(pcapFile.close());
}

#endif // GTEST
