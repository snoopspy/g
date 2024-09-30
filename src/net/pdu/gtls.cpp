#include "gtls.h"

// ----------------------------------------------------------------------------
// GTlsHdr
// ----------------------------------------------------------------------------
GTls::Record* GTls::Record::check(gbyte* p, uint32_t* size) {
	if (*size < sizeof(GTls::Record)) {
		qDebug() << QString("size(%d) is too short").arg(*size); // gilgil temp 2024.10.01
		return nullptr;
	}
	GTls::Record* tr = GTls::PRecord(p);
	uint16_t r = sizeof(GTls::Record) + tr->length();
	if (*size < r) {
		qDebug() << QString("size(%d) is less then r(%d)").arg(*size).arg(r); // gilgil temp 2024.10.01
		return nullptr;
	}
	*size -= sizeof(GTls::Record);
	return tr;
}

GTls::Handshake* GTls::Handshake::check(GTls::Record* tr, uint32_t* size) {
	Q_ASSERT(tr->contentType() == GTls::Record::Handshake);
	if (*size < sizeof(GTls::Record) + sizeof(GTls::Handshake)) {
		qDebug() << QString("size(%d) is too short").arg(*size); // gilgil temp 2024.10.01
		return nullptr;
	}
	GTls::Handshake* hs = GTls::PHandshake(pbyte(tr) + sizeof(GTls::Record));
	uint16_t length = hs->length_;
	uint16_t r = sizeof(GTls::Handshake) + length;
	if (*size < r) {
		qDebug() << QString("size(%d) is less then r(%d)").arg(*size).arg(r); // gilgil temp 2024.10.01
		return nullptr;
	}
	*size -= sizeof(GTls::Handshake);
	return hs;
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
		QObject::connect(&certMgr_, &GCertMgr::managed, this, &GTlsTestObj::debug);
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

public slots:
	void debug(GTls::Handshake* hs) {
		uint8_t hst = hs->handshakeType_;
		printf("handshakeType=%d 0x%02x\n", hst, hst);
	}
};

TEST(GTlsTest, netclient_g_gilgil_net_tls1_0) {
	GTlsTestObj obj("pcap/tls/netclient-g.gilgil.net-tls1_0.pcap");
	EXPECT_EQ(obj.open(), true);
	obj.readOnePacket();
}

#include "gtls.moc"

#endif // GTEST
