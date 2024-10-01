#include "gcertmgr.h"

// ----------------------------------------------------------------------------
// GCertMgr
// ----------------------------------------------------------------------------
bool GCertMgr::doOpen() {
	if (tcpFlowMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "tcpFlowMgr is null");
		return false;
	}
	tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request(this, sizeof(Item));
	tcpFlowMgr_->managables_.insert(this);

	return true;
}

bool GCertMgr::doClose() {
	return true;
}

void GCertMgr::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	qDebug() << QString("_tcpFlowDetected %1").arg(QString(tcpFlowKey)); // gilgil temp 2021.04.07
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	GTcpHdr* tcpHdr = tcpFlowMgr_->currentPacket_->tcpHdr_;
	Q_ASSERT(tcpHdr != nullptr);
	bool synExist = (tcpHdr->flags() & GTcpHdr::Syn) != 0;
	bool finExist = (tcpHdr->flags() & GTcpHdr::Fin) != 0;
	new (item) Item(tcpHdr->seq() + (synExist || finExist ? 1 : 0));
}

void GCertMgr::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	item->~Item();
	qDebug() << QString("_tcpFlowDeleted %1").arg(QString(tcpFlowKey)); // gilgil temp 2021.04.07
}

void GCertMgr::manage(GPacket* packet) {
	Item* item = getItem(tcpFlowMgr_->currentTcpFlowVal_);
	Q_ASSERT(item != nullptr);
	if (item->handshakeFinished_) return;

	GBuf tcpData = packet->tcpData_;
	if (!tcpData.valid()) return;
	QByteArray ba(pchar(tcpData.data_), tcpData.size_);
	item->segment_ += ba;

	while (true) {
		gbyte* p = pbyte(item->segment_.constData());
		uint32_t size = item->segment_.size();
		if (size == 0) break;

		GTls::Record* tr = GTls::Record::check(p, &size);
		if (tr == nullptr) return;
		if (tr->contentType_ != GTls::Record::Handshake) {
			item->handshakeFinished_ = true; // gilgil temp 2024.10.01
			return;
		}

		GTls::Handshake* hs = GTls::Handshake::check(tr, &size);
		if (hs == nullptr) return;
		emit managed(hs);

		uint32_t length = hs->length_;
		switch (hs->handshakeType_) {
			case GTls::Handshake::HelloRequest: qDebug() << "HelloRequest" << length; break;
				case GTls::Handshake::ClientHello: qDebug() << "ClientHello" << length; break;
				case GTls::Handshake::ServerHello: qDebug() << "ServerHello" << length; break;
				case GTls::Handshake::NetSessionTicket: qDebug() << "NetSessionTicket" << length; break;
				case GTls::Handshake::Certificate: qDebug() << "Certificate" << length; break;
				case GTls::Handshake::ServerKeyExchange: qDebug() << "ServerKeyExchange" << length; break;
				case GTls::Handshake::CertificateRequest: qDebug() << "CertificateRequest" << length; break;
				case GTls::Handshake::ServerHelloDone: qDebug() << "ServerHelloDone" << length; break;
				case GTls::Handshake::CertificateVerify: qDebug() << "CertificateVerify" << length; break;
				case GTls::Handshake::ClientKeyExchange: qDebug() << "ClientKeyExchange" << length; break;
				case GTls::Handshake::Finished: qDebug() << "Finished" << length; break;
				case GTls::Handshake::CertificateStatus: qDebug() << "CertificateStatus" << length; break;
			default:
				qDebug() << "Unknown" << uint(hs->handshakeType_) << length; break;
		}

		if (hs->handshakeType_ == GTls::Handshake::ClientHello) {
			GTls::ClientHelloHs ch;
			ch.parse(hs);
			qDebug() << ch.version_;
		}
		item->segment_.remove(0, sizeof(GTls::Record) + sizeof(GTls::Handshake) + length);
	}
}
