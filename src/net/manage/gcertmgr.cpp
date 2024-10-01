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

QString GCertMgr::extractServerName(GTls::Handshake *hs) {
	GTls::ClientHelloHs ch;
	ch.parse(hs);
	if (ch.extensions_.size() > 0) {
		GTls::Extension* extension = ch.extensions_.at(0);
		uint16_t type = extension->type();
		if (type == GTls::Extension::ServerName) {
			GTls::ServerNameIndication* sni = GTls::PServerNameIndication(extension);
			QByteArray serverName = sni->serverName();
			qDebug() << QString("serverName=%1").arg(serverName);
			return serverName;
		} else {
			qWarning() << QString("Invalid extension type(%1)").arg(type);
			return QString();
		}
	}
}

QList<QByteArray> GCertMgr::extractCertificates(GTls::Handshake *hs) {
	GTls::CertificateHs cr;
	cr.parse(hs);
	return cr.certificates_;
}

void GCertMgr::manage(GPacket* packet) {
	Item* item = getItem(tcpFlowMgr_->currentTcpFlowVal_);
	Q_ASSERT(item != nullptr);
	if (!item->checkNeeded_) return;

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
			item->checkNeeded_ = false;
			return;
		}

		GTls::Handshake* hs = GTls::Handshake::check(tr, &size);
		if (hs == nullptr) return;
		emit handshakeDetected(hs);

		uint32_t length = hs->length_;
		uint8_t handshakeType = hs->handshakeType_;
		switch (handshakeType) {
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

		switch (handshakeType) {
			case GTls::Handshake::ClientHello:
				item->serverName_ =	extractServerName(hs);
				item->checkNeeded_ = false;
				break;
			case GTls::Handshake::ServerHello: {
				GTls::ServerHelloHs sh;
				sh.parse(hs); // for debug
				break;
			}
			case GTls::Handshake::Certificate: {
				item->certificates_ = extractCertificates(hs);
				item->checkNeeded_ = false;

				Item* revItem = getItem(tcpFlowMgr_->currentRevTcpFlowVal_);
				if (revItem != nullptr) {
					emit certificatesDetected(revItem->serverName_, item->certificates_);
				}
				break;
			}
			default:
		}
		item->segment_.remove(0, sizeof(GTls::Record) + sizeof(GTls::Handshake) + length);
	}
}
