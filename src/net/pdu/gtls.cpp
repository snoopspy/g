#include "gtls.h"

// ----------------------------------------------------------------------------
// GTlsHdr
// ----------------------------------------------------------------------------
GTls::Record* GTls::Record::check(gbyte* p, uint32_t* size) {
	if (*size < sizeof(GTls::Record)) {
		qDebug() << QString("size(%1) is too short").arg(*size); // gilgil temp 2024.10.01
		return nullptr;
	}
	GTls::Record* tr = GTls::PRecord(p);
	uint16_t r = sizeof(GTls::Record) + tr->length();
	if (*size < r) {
		qDebug() << QString("size(%1) is less then r(%2)").arg(*size).arg(r); // gilgil temp 2024.10.01
		return nullptr;
	}
	*size -= sizeof(GTls::Record);
	return tr;
}

GTls::Handshake* GTls::Handshake::check(GTls::Record* tr, uint32_t* size) {
	Q_ASSERT(tr->contentType() == GTls::Record::Handshake);
	if (*size < sizeof(GTls::Handshake)) {
		qDebug() << QString("size(%1) is too short").arg(*size); // gilgil temp 2024.10.01
		return nullptr;
	}
	GTls::Handshake* hs = GTls::PHandshake(pbyte(tr) + sizeof(GTls::Record));
	uint16_t length = hs->length_;
	uint16_t r = sizeof(GTls::Handshake) + length;
	if (*size < r) {
		qDebug() << QString("size(%1) is less then r(%2)").arg(*size).arg(r); // gilgil temp 2024.10.01
		return nullptr;
	}
	*size -= sizeof(GTls::Handshake);
	return hs;
}

bool GTls::ClientHelloHs::parse(GTls::Handshake* hs) {
	*PHandshake(this) = *hs;
	//handshakeType_ = hs->handshakeType_;
	//uint32_t len = length_ = hs->length_;

	//gbyte* p = pbyte(hs);
	//p += sizeof(GTls::Handshake); len -= sizeof(GTls::Handshake);
	//version_ = ntohs(*puint16_t(p)); p += sizeof(version_); len -= sizeof(version_);
//	memcpy(random_, p, sizeof(random_)); p += sizeof(random_); len -= sizeof(random_);
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>
#include <GSyncPcapFile>
#include <GTcpFlowMgr>
#include <GCertMgr>

struct GTlsTestObj : GObj {
	Q_OBJECT
	GSyncPcapFile pcapFile_;
	GTcpFlowMgr tcpFlowMgr_;
	GCertMgr certMgr_;

public:
	GTlsTestObj(QString fileName) {
		pcapFile_.fileName_ = fileName;
		QObject::connect(&certMgr_, &GCertMgr::managed, this, &GTlsTestObj::debug, Qt::DirectConnection);
		certMgr_.tcpFlowMgr_ = &tcpFlowMgr_;
	}

	~GTlsTestObj() {
		pcapFile_.close();
		tcpFlowMgr_.close();
		certMgr_.close();
	}

	bool open() {
		if (!pcapFile_.open()) return false;

		GEthPacket packet;

		if (pcapFile_.read(&packet) != GPacket::Ok) return false; // SYN
		if (packet.tcpHdr_ == nullptr) return false;
		if (packet.tcpHdr_->flags() != GTcpHdr::Syn) return false;

		if (pcapFile_.read(&packet) != GPacket::Ok) return false; // SYN + ACK
		if (packet.tcpHdr_ == nullptr) return false;
		if (packet.tcpHdr_->flags() != (GTcpHdr::Syn | GTcpHdr::Ack)) return false;

		if (pcapFile_.read(&packet) != GPacket::Ok) return false; // ACK
		if (packet.tcpHdr_ == nullptr) return false;
		if (packet.tcpHdr_->flags() != GTcpHdr::Ack) return false;

		if (!tcpFlowMgr_.open()) return false;
		if (!certMgr_.open()) return false;
		return true;
	}

	void readOnePacket() {
		GEthPacket packet;
		GPacket::Result res = pcapFile_.read(&packet);
		if (res != GPacket::Ok) return
				;
		tcpFlowMgr_.manage(&packet);
		certMgr_.manage(&packet);
	}

	QList<GTls::Handshake*> hsList_;
public slots:
	void debug(GTls::Handshake* hs) {
		hsList_.push_back(hs);
	}
};

TEST(GTlsTest, netclient_g_gilgil_net_tls1_0) {
	GTlsTestObj obj("pcap/tls/netclient-g.gilgil.net-tls1_0.pcap");
	EXPECT_EQ(obj.open(), true);

	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 1);
	GTls::Handshake* hs = obj.hsList_.at(0);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ClientHello);
	EXPECT_EQ(hs->length_, 186);
	obj.hsList_.clear();
}

TEST(GTlsTest, netclient_g_gilgil_net_tls1_1) {
	GTlsTestObj obj("pcap/tls/netclient-g.gilgil.net-tls1_1.pcap");
	EXPECT_EQ(obj.open(), true);

	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 1);
	GTls::Handshake* hs = obj.hsList_.at(0);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ClientHello);
	EXPECT_EQ(hs->length_, 186);

	obj.readOnePacket();
	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 4);
	hs = obj.hsList_.at(0);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ServerHello);
	EXPECT_EQ(hs->length_, 57);
	hs = obj.hsList_.at(1);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::Certificate);
	EXPECT_EQ(hs->length_, 817);
	hs = obj.hsList_.at(2);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ServerKeyExchange);
	EXPECT_EQ(hs->length_, 331);
	hs = obj.hsList_.at(3);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ServerHelloDone);
	EXPECT_EQ(hs->length_, 4);
}

TEST(GTlsTest, netclient_g_gilgil_net_tls1_2) {
	GTlsTestObj obj("pcap/tls/netclient-g.gilgil.net-tls1_2.pcap");
	EXPECT_EQ(obj.open(), true);

	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 1);
	GTls::Handshake* hs = obj.hsList_.at(0);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ClientHello);
	EXPECT_EQ(hs->length_, 508);

	obj.readOnePacket();
	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 4);
	hs = obj.hsList_.at(0);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ServerHello);
	EXPECT_EQ(hs->length_, 508);
	hs = obj.hsList_.at(1);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::Certificate);
	EXPECT_EQ(hs->length_, 813);

	hs = obj.hsList_.at(2);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ServerKeyExchange);
	EXPECT_EQ(hs->length_, 329);

	hs = obj.hsList_.at(3);
	EXPECT_EQ(hs->handshakeType_, GTls::Handshake::ServerHelloDone);
	EXPECT_EQ(hs->length_, 0);
}

#include "gtls.moc"

#endif // GTEST
