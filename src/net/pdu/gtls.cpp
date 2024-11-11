#include "gtls.h"

// ----------------------------------------------------------------------------
// GTlsHdr
// ----------------------------------------------------------------------------
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

void GTls::ClientHelloHs::parse(GTls::Handshake* hs) {
	*PHandshake(this) = *hs;

	gbyte* p = pbyte(hs); uint32_t len = length_; gbyte* end = p + sizeof(GTls::Handshake) + len;
	p += sizeof(GTls::Handshake);

	version_ = ntohs(*puint16_t(p)); p += sizeof(version_); len -= sizeof(version_);

	memcpy(random_, p, sizeof(random_)); p += sizeof(random_); len -= sizeof(random_);

	uint8_t sessionIdLength = *puint8_t(p); p += sizeof(sessionIdLength); len -= sizeof(sessionIdLength);
	if (sessionIdLength > 32)
		qWarning() << QString("too big sessionIdLengh(%1)").arg(sessionIdLength);
	sessionId_ = QByteArray(pchar(p), sessionIdLength); p += sessionIdLength; len -= sessionIdLength;

	uint16_t cipherSuitesLength = ntohs(*puint16_t(p)); p += sizeof(cipherSuitesLength); len -= sizeof(cipherSuitesLength);
	if (cipherSuitesLength > 256)
		qWarning() << QString("too big cipherSuitesLength(%1)").arg(cipherSuitesLength);
	for (int i = 0; i < cipherSuitesLength / 2; i++) {
		uint16_t cipherSuite = ntohs(*puint16_t(p)); p += sizeof(cipherSuite); len -= sizeof(cipherSuite);
		cipherSuites_.push_back(cipherSuite);
	}

	uint8_t compressionMethodsLength = *puint8_t(p); p += sizeof(compressionMethodsLength); len -= sizeof(compressionMethodsLength);
	if (compressionMethodsLength != 0 && compressionMethodsLength != 1)
		qWarning() << QString("Invalid compressionMethodsLength(%1)").arg(compressionMethodsLength);
	compressionMethod_ = QByteArray(pchar(p), compressionMethodsLength);  p += compressionMethodsLength; len -= compressionMethodsLength;

	int16_t extensionsLength = ntohs(*pint16_t(p)); p += sizeof(extensionsLength); len -= sizeof(extensionsLength);
	if (extensionsLength > 8192)
		qWarning() << QString("too big extensionsLength(%1)").arg(extensionsLength);
	while (p < end) {
		Extension* extension = PExtension(p);
		extensions_.push_back(extension);

		uint16_t oneLength = extension->length(); oneLength += sizeof(Extension);
		p += oneLength; len -= oneLength;
		extensionsLength -= oneLength;
		if (extensionsLength < 0) {
			qWarning() << QString("Invalid extensionsLength(%1)").arg(extensionsLength);
			break;
		}
	}
	if (extensionsLength != 0)
		qWarning() << QString("extensionsLength is not zero(%1)").arg(extensionsLength);

	if (len != 0)
		qWarning() << QString("len is not zero(%1)").arg(len);
}

void GTls::ServerHelloHs::parse(GTls::Handshake* hs) {
	*PHandshake(this) = *hs;

	gbyte* p = pbyte(hs); uint32_t len = length_; gbyte* end = p + sizeof(GTls::Handshake) + len;
	p += sizeof(GTls::Handshake);

	version_ = ntohs(*puint16_t(p)); p += sizeof(version_); len -= sizeof(version_);

	memcpy(random_, p, sizeof(random_)); p += sizeof(random_); len -= sizeof(random_);

	uint8_t sessionIdLength = *puint8_t(p); p += sizeof(sessionIdLength); len -= sizeof(sessionIdLength);
	if (sessionIdLength > 32)
		qWarning() << QString("too big sessionIdLengh(%1)").arg(sessionIdLength);
	sessionId_ = QByteArray(pchar(p), sessionIdLength); p += sessionIdLength; len -= sessionIdLength;

	cipherSuite_ = ntohs(*puint16_t(p)); p += sizeof(cipherSuite_); len -= sizeof(cipherSuite_);

	uint8_t compressionMethodsLength = *puint8_t(p); p += sizeof(compressionMethodsLength); len -= sizeof(compressionMethodsLength);
	if (compressionMethodsLength != 0 && compressionMethodsLength != 1)
		qWarning() << QString("Invalid compressionMethodsLength(%1)").arg(compressionMethodsLength);
	compressionMethod_ = QByteArray(pchar(p), compressionMethodsLength);  p += compressionMethodsLength; len -= compressionMethodsLength;

	int16_t extensionsLength = ntohs(*pint16_t(p)); p += sizeof(extensionsLength); len -= sizeof(extensionsLength);
	if (extensionsLength > 8192)
		qWarning() << QString("too big extensionsLength(%1)").arg(extensionsLength);
	while (p < end) {
		Extension* extension = PExtension(p);
		extensions_.push_back(extension);

		uint16_t oneLength = extension->length(); oneLength += sizeof(Extension);
		p += oneLength; len -= oneLength;
		extensionsLength -= oneLength;
		if (extensionsLength < 0) {
			qWarning() << QString("invalid extensionsLength(%1)").arg(extensionsLength);
			break;
		}
	}
	if (extensionsLength != 0)
		qWarning() << QString("extensionsLength is not zero(%1)").arg(extensionsLength);

	if (len != 0)
		qWarning() << QString("len is not zero(%1)").arg(len);
}

void GTls::CertificateHs::parse(GTls::Handshake* hs) {
	*PHandshake(this) = *hs;

	gbyte* p = pbyte(hs); uint32_t len = length_; gbyte* end = p + sizeof(GTls::Handshake) + len;
	p += sizeof(GTls::Handshake);

	certificatesLength_ = *PLength3(p); p += sizeof(certificatesLength_); len -= sizeof(certificatesLength_);
	int32_t certificatesLength = certificatesLength_;
	while (p < end) {
		Length3 oneLength = *PLength3(p); p += sizeof(oneLength); len -= sizeof(oneLength);
		oneLength.len_[0] = 0xFF; oneLength.len_[1] = 0xFF; oneLength.len_[2] = 0xFF;
		qsizetype ol = oneLength;
		if (ol < 0 || ol > 65536) {
			qWarning() << QString("too big certificat size(%1 %2)").arg(oneLength).arg(ol);
			return;
		}
		QByteArray certificate(pchar(p), ol);
		certificates_.push_back(certificate);
		p += oneLength; len -= oneLength;
		certificatesLength -= sizeof(Length3) + oneLength;
		if (certificatesLength < 0) {
			qWarning() << QString("Invalid certificatesLength(%1)").arg(certificatesLength);
			break;
		}
	}
	if (certificatesLength != 0)
		qWarning() << QString("extensionsLength is not zero(%1)").arg(certificatesLength);

	if (len != 0)
		qWarning() << QString("len is not zero(%1)").arg(len);
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
		QObject::connect(&certMgr_, &GCertMgr::handshakeDetected, this, &GTlsTestObj::doHandshakeDetected, Qt::DirectConnection);
		QObject::connect(&certMgr_, &GCertMgr::certificatesDetected, this, &GTlsTestObj::doCertificatesDetected, Qt::DirectConnection);
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

	QList<GTls::Handshake> hsList_;
public slots:
	void doHandshakeDetected(GTls::Handshake* hs) {
		hsList_.push_back(*hs);
	}
	void doCertificatesDetected(QString serverName, struct timeval st, QList<QByteArray> certs) {
		qDebug() << serverName << certs.size();
	}
};

TEST(GTlsTest, netclient_g_gilgil_net_tls1_0) {
	GTlsTestObj obj("pcap/tls/netclient-g.gilgil.net-tls1_0.pcap");
	EXPECT_EQ(obj.open(), true);

	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 1);
	GTls::Handshake hs = obj.hsList_.at(0);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ClientHello);
	EXPECT_EQ(hs.length_, 186);
	obj.hsList_.clear();
}

TEST(GTlsTest, netclient_g_gilgil_net_tls1_1) {
	GTlsTestObj obj("pcap/tls/netclient-g.gilgil.net-tls1_1.pcap");
	EXPECT_EQ(obj.open(), true);

	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 1);
	GTls::Handshake hs = obj.hsList_.at(0);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ClientHello);
	EXPECT_EQ(hs.length_, 186);
	obj.hsList_.clear();

	obj.readOnePacket();
	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 4);
	hs = obj.hsList_.at(0);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ServerHello);
	EXPECT_EQ(hs.length_, 57);
	hs = obj.hsList_.at(1);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::Certificate);
	EXPECT_EQ(hs.length_, 813);
	hs = obj.hsList_.at(2);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ServerKeyExchange);
	EXPECT_EQ(hs.length_, 327);
	hs = obj.hsList_.at(3);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ServerHelloDone);
	EXPECT_EQ(hs.length_, 0);
}

TEST(GTlsTest, netclient_g_gilgil_net_tls1_2) {
	GTlsTestObj obj("pcap/tls/netclient-g.gilgil.net-tls1_2.pcap");
	EXPECT_EQ(obj.open(), true);

	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 1);
	GTls::Handshake hs = obj.hsList_.at(0);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ClientHello);
	EXPECT_EQ(hs.length_, 508);
	obj.hsList_.clear();

	obj.readOnePacket();
	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 4);
	hs = obj.hsList_.at(0);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ServerHello);
	EXPECT_EQ(hs.length_, 57);
	hs = obj.hsList_.at(1);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::Certificate);
	EXPECT_EQ(hs.length_, 813);

	hs = obj.hsList_.at(2);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ServerKeyExchange);
	EXPECT_EQ(hs.length_, 329);

	hs = obj.hsList_.at(3);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ServerHelloDone);
	EXPECT_EQ(hs.length_, 0);
}

TEST(GTlsTest, netclient_g_gilgil_net_tls1_3) {
	GTlsTestObj obj("pcap/tls/netclient-g.gilgil.net-tls1_3.pcap");
	EXPECT_EQ(obj.open(), true);

	obj.readOnePacket();
	ASSERT_EQ(obj.hsList_.size(), 1);
	GTls::Handshake hs = obj.hsList_.at(0);
	EXPECT_EQ(hs.handshakeType_, GTls::Handshake::ClientHello);
	EXPECT_EQ(hs.length_, 250);
	obj.hsList_.clear();

	obj.readOnePacket();
	obj.readOnePacket();
}

#include "gtls.moc"

#endif // GTEST
