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
	QByteArray ba(reinterpret_cast<const char*>(tcpData.data_), tcpData.size_);
	*item += ba;

	gbyte* p = pbyte(item->constData());
	uint32_t size = item->size();
	GTls::Record* tr = GTls::Record::check(p, &size);
	if (tr == nullptr) return;
	if (tr->contentType_ != GTls::Record::Handshake) {
		item->handshakeFinished_ = true;
		return;
	}

	GTls::Handshake* hs = GTls::Handshake::check(tr, &size);
	if (hs == nullptr) return;

	uint32_t length = hs->length_;
	switch (hs->handshakeType_) {
		case GTls::Handshake::ClientHello: printf("ClientHello %u\n", length); break;
		case GTls::Handshake::ServerHello: printf("ServerHello %u\n", length); break;
		case GTls::Handshake::Certificate: printf("Certificate %u\n", length); break;
		default:
			printf("%u %u", hs->handshakeType_, length);
	}
}
