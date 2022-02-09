#include "gradiohdr.h"

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
	{2, 2}, // 8, TxAttenuation (u16)
	{2, 2}, // 9, DbTxAttenuation (u16)
	{1, 1}, // 10, DbmTxPower (s8)
	{1, 1}, // 11, Antenna (u8)
	{1, 1}, // 12, DbAntennaSignal (u8)
	{1, 1}, // 13, DbAntennaNoise (u8)
	{2, 2}, // 14, RxFlags (u16)
	{2, 2}, // 15, TxFlags (u16 flags)
	{1, 1}, // 16, RtsRetries (u8 retries)
	{1, 1}, // 17, DataRetries (u8 retries)
	{4, 4}, // 18, XChannel (u32 flags, u16 freq, u8 channel, u8 maxpower)
	{3, 1}, // 19, Mcs (u8 known, u8 flags, u8 mcs)
	{8, 4}, // 20, AMpdu (u32 reference number, u16 flags, u8 delimiter CRC value, u8 reserved)
	{16, 2}, // 21, Vht (u16 known, u8 flags, u8 bandwidth, u8 mcs_nss[4], u8 coding, u8 group_id, u16 partial_aid)
	{12, 8}, // 22, Times (u64 timestamp, u16 accuracy, u8 unit/position, u8 flags)
	{12, 2}, // 23, He (u16 data1, data2, data3, data4, data5, data6)
	{12, 2}, // 24, HeMu (u16 flags1, u16 flags2, u8 RU_channel1[4], u8 RU_channel2[4])
	{6, 2}, // 25, HeMuOtherUser (u16 per_user_1, per_user_2, u8 per_user_position, per_user_known)
	{1, 1}, // 26, ZeroLenghPsdu (u8 type)
	{4, 2}, // 27, LSig (u16 data1, data2)
	{4, 4}, // 28, Tlv (list of TLVs detailed below)
	{0, 0}, // 29, RadiotapNamespace (no contents)
	{6, 2}, // 30, VendorNamespace (u8 OUI[3], u8 sub_namespace, u16 skip_length)
	{0, 0}, // 31, Ext
};

// ----------------------------------------------------------------------------
// GRadioHdr
// ----------------------------------------------------------------------------
GRadioHdr* GRadioHdr::check(gbyte* p, uint32_t size) {
	GRadioHdr* radioHdr = PRadioHdr(p);
	le16_t len = radioHdr->len_;
	if (len < sizeof(GRadioHdr) || len > size) {
		qWarning() << QString("invalid size radioHdr->len=%1(0x%2) size=%3").arg(len).arg(len).arg(size);
		return nullptr;
	}
	return radioHdr;
}

void GRadioHdr::init() {
	ver_ = 0;
	pad_ = 0;
	len_ = sizeof(GRadioHdr);
	present_.p_ = 0;
}

QList<GBuf> GRadioHdr::presentInfo(BitNo bitNo) {
	QList<GBuf> res;

	int offset = sizeof(GRadioHdr);

	Present* present = &present_;
	int presentFlagCount = 1;
	while (true) {
		present = present->next();
		if (present == nullptr)
			break;
		offset += sizeof(Present);
		presentFlagCount++;
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
					gbyte* p = pbyte(this) + offset;
					GBuf buf(p, size);
					res.push_back(buf);
				}
				offset += size;
			}
			mask <<= 1;
		}
		present = present->next();
	}

	return res;
}

int GRadioHdr::freqToChannel(int freq) {
	if (freq >= 2412 && freq <= 2484) {
		if (freq % 5 != 2) {
			qWarning() << QString("freq(%1) must be end with 2 or 7").arg(freq);
			return -1;
		}
		return (freq - 2412) / 5 + 1;
	}

	if (freq >= 5160 && freq <= 5980) {
		if (freq % 5 != 0) {
			qWarning() << QString("freq(%1) must be multiple of 5").arg(freq);
			return -1;
		}
		return (freq - 5160) / 5 + 32;
	}
	qWarning() << QString("invalid freq %1").arg(freq);
	return -1;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

#include <GSyncPcapFile>
TEST(RadioHdr, beacon_a2000ua_testap5g_pcap) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/dot11/beacon-a2000ua-testap5g.pcap";
	EXPECT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Dot11);

	GDot11Packet packet;
	EXPECT_EQ(pcapFile.read(&packet), GPacket::Ok);

	GRadioHdr* radioHdr = packet.radioHdr_;
	EXPECT_NE(radioHdr, nullptr);
	EXPECT_EQ(radioHdr->len_, 32);
	EXPECT_EQ(le32_t(radioHdr->present_), 0xa00040ae);
	EXPECT_EQ(le32_t(*radioHdr->present_.next()), 0xa0000820);
	EXPECT_EQ(le32_t(*radioHdr->present_.next()->next()), 0x00000820);

	QList<GBuf> ba;

	ba = radioHdr->presentInfo(GRadioHdr::Flags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x10", 1), 0);

	ba = radioHdr->presentInfo(GRadioHdr::Rate);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x0c", 1), 0); // 500Kbps * 12 == 6.0 Mb/s

	ba = radioHdr->presentInfo(GRadioHdr::Channel);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 4);
	EXPECT_EQ(memcmp(ba[0].data_, "\x3c\x14\x40\x01", 4), 0); // 5180, 0x4001

	ba = radioHdr->presentInfo(GRadioHdr::AntennaSignal);
	EXPECT_EQ(ba.count(), 3);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\xf7", 1), 0); // -9 -dBm
	EXPECT_EQ(memcmp(ba[1].data_, "\xf6", 1), 0); // -10 -dBm
	EXPECT_EQ(memcmp(ba[2].data_, "\xf6", 1), 0); // -10 -dBm

	ba = radioHdr->presentInfo(GRadioHdr::LockQuality);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 2);
	EXPECT_EQ(memcmp(ba[0].data_, "\x4b\x00", 2), 0); // 75

	ba = radioHdr->presentInfo(GRadioHdr::Antenna);
	EXPECT_EQ(ba.count(), 2);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x00", 1), 0); // 0
	EXPECT_EQ(memcmp(ba[1].data_, "\x01", 1), 0); // 1

	ba = radioHdr->presentInfo(GRadioHdr::RxFlags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 2);
	EXPECT_EQ(memcmp(ba[0].data_, "\x00\x00", 2), 0); // 0x0000

	EXPECT_TRUE(pcapFile.close());
}

TEST(RadioHdr, beacon_awus051nh_testap5g_pcap) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/dot11/beacon-awus051nh-testap5g.pcap";
	EXPECT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Dot11);

	GDot11Packet packet;
	EXPECT_EQ(pcapFile.read(&packet), GPacket::Ok);

	GRadioHdr* radioHdr = packet.radioHdr_;
	EXPECT_NE(radioHdr, nullptr);
	EXPECT_EQ(radioHdr->len_, 18);
	EXPECT_EQ(le32_t(radioHdr->present_), 0x0000482e);

	QList<GBuf> ba;

	ba = radioHdr->presentInfo(GRadioHdr::Flags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x00", 1), 0); // 0x00

	ba = radioHdr->presentInfo(GRadioHdr::Rate);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x0c", 1), 0); // 500Kbps * 12 == 6.0 Mb/s

	ba = radioHdr->presentInfo(GRadioHdr::Channel);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 4);
	EXPECT_EQ(memcmp(ba[0].data_, "\x3c\x14\x40\x01", 4), 0); // 5180, 0x4001

	ba = radioHdr->presentInfo(GRadioHdr::AntennaSignal);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\xef", 1), 0); // -17 dBm

	ba = radioHdr->presentInfo(GRadioHdr::Antenna);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x01", 1), 0); // 1

	ba = radioHdr->presentInfo(GRadioHdr::RxFlags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 2);
	EXPECT_EQ(memcmp(ba[0].data_, "\x00\x00", 2), 0); // 0x0000

	EXPECT_TRUE(pcapFile.close());
}

TEST(RadioHdr, beacon_forcerecon_testap5g_pcap) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/dot11/beacon-forcerecon-testap5g.pcap";
	EXPECT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Dot11);

	GDot11Packet packet;
	EXPECT_EQ(pcapFile.read(&packet), GPacket::Ok);

	GRadioHdr* radioHdr = packet.radioHdr_;
	EXPECT_NE(radioHdr, nullptr);
	EXPECT_EQ(radioHdr->len_, 54);
	EXPECT_EQ(le32_t(radioHdr->present_), 0xa040402f);
	EXPECT_EQ(le32_t(*radioHdr->present_.next()), 0x00000820);

	QList<GBuf> ba;

	ba = radioHdr->presentInfo(GRadioHdr::Tsft);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 8);
	EXPECT_EQ(memcmp(ba[0].data_, "\x8c\x48\x6b\x06\x00\x00\x00\x00", 8), 0); // 107694220

	ba = radioHdr->presentInfo(GRadioHdr::Flags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x10", 1), 0); // 0x10

	ba = radioHdr->presentInfo(GRadioHdr::Rate);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x0c", 1), 0); // 500Kbps * 12 == 6.0 Mb/s

	ba = radioHdr->presentInfo(GRadioHdr::Channel);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 4);
	EXPECT_EQ(memcmp(ba[0].data_, "\x3c\x14\x40\x01", 1), 0); // 5180, 0x4001

	ba = radioHdr->presentInfo(GRadioHdr::AntennaSignal);
	EXPECT_EQ(ba.count(), 2);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\xe4", 1), 0); // -28 dBm
	EXPECT_EQ(memcmp(ba[1].data_, "\xe4", 1), 0); // -28 dBm

	ba = radioHdr->presentInfo(GRadioHdr::RxFlags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 2);
	EXPECT_EQ(memcmp(ba[0].data_, "\x00\x00", 2), 0); // 0x0000

	ba = radioHdr->presentInfo(GRadioHdr::Timestamp);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 12);
	EXPECT_EQ(memcmp(ba[0].data_, "\x76\x48\x6b\x06\x00\x00\x00\x00\x16\x00\x11\x03", 12), 0);

	ba = radioHdr->presentInfo(GRadioHdr::Antenna);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x00", 1), 0); // 0

	EXPECT_TRUE(pcapFile.close());
}

TEST(RadioHdr, beacon_galaxy7_testap5g_pcap) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/dot11/beacon-galaxy7-testap5g.pcap";
	EXPECT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Dot11);

	GDot11Packet packet;
	EXPECT_EQ(pcapFile.read(&packet), GPacket::Ok);

	GRadioHdr* radioHdr = packet.radioHdr_;
	EXPECT_NE(radioHdr, nullptr);
	EXPECT_EQ(radioHdr->len_, 24);
	EXPECT_EQ(le32_t(radioHdr->present_), 0x0000006b);

	QList<GBuf> ba;

	ba = radioHdr->presentInfo(GRadioHdr::Tsft);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 8);
	EXPECT_EQ(memcmp(ba[0].data_, "\x7c\x55\xc3\x5f\x00\x00\x00\x00", 8), 0); // 1606636924

	ba = radioHdr->presentInfo(GRadioHdr::Flags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x10", 1), 0); // 0x10

	ba = radioHdr->presentInfo(GRadioHdr::Channel);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 4);
	EXPECT_EQ(memcmp(ba[0].data_, "\x3c\x14\x00\x00", 4), 0); // 5180, 0x0000

	ba = radioHdr->presentInfo(GRadioHdr::AntennaSignal);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\xe3", 1), 0); // -29 dBm

	ba = radioHdr->presentInfo(GRadioHdr::AntennaNoise);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\xa4", 1), 0); // -92 dBm

	EXPECT_TRUE(pcapFile.close());
}

TEST(RadioHdr, beacon_nexus5_testap5g_pcap) {
	GSyncPcapFile pcapFile;
	pcapFile.fileName_ = "pcap/dot11/beacon-nexus5-testap5g.pcap";
	EXPECT_TRUE(pcapFile.open());
	EXPECT_EQ(pcapFile.dlt(), GPacket::Dot11);

	GDot11Packet packet;
	EXPECT_EQ(pcapFile.read(&packet), GPacket::Ok);

	GRadioHdr* radioHdr = packet.radioHdr_;
	EXPECT_NE(radioHdr, nullptr);
	EXPECT_EQ(radioHdr->len_, 40);
	EXPECT_EQ(le32_t(radioHdr->present_), 0x4008006f);

	QList<GBuf> ba;

	ba = radioHdr->presentInfo(GRadioHdr::Tsft);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 8);
	EXPECT_EQ(memcmp(ba[0].data_, "\xd7\x00\x2e\xb5\x00\x00\x00\x00", 8), 0); // 3039690967

	ba = radioHdr->presentInfo(GRadioHdr::Flags);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x10", 1), 0); // 0x10

	ba = radioHdr->presentInfo(GRadioHdr::Rate);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\x0c", 1), 0); // 500Kbps * 12 == 6.0 Mb/s

	ba = radioHdr->presentInfo(GRadioHdr::Channel);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 4);
	EXPECT_EQ(memcmp(ba[0].data_, "\x3c\x14\x40\x01", 4), 0); // 5180, 0x4001

	ba = radioHdr->presentInfo(GRadioHdr::AntennaSignal);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\xe5", 1), 0); // -27 dBm

	ba = radioHdr->presentInfo(GRadioHdr::AntennaNoise);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 1);
	EXPECT_EQ(memcmp(ba[0].data_, "\xa4", 1), 0); // -92 dBm

	ba = radioHdr->presentInfo(GRadioHdr::Mcs);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 3);
	EXPECT_EQ(memcmp(ba[0].data_, "\x00\x00\x00", 3), 0); // 0x000000

	ba = radioHdr->presentInfo(GRadioHdr::VendorNamespace);
	EXPECT_EQ(ba.count(), 1);
	EXPECT_EQ(ba[0].size_, 6);
	EXPECT_EQ(memcmp(ba[0].data_, "\x4e\x45\x58\x00\x06\x00", 6), 0); // cb3002000000 not checked

	EXPECT_TRUE(pcapFile.close());
}

#endif // GTEST
