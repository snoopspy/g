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

	if (saveCertFile_) {
		if (!makeFolder(saveCertFileFolder_)) {
			SET_ERR(GErr::Fail, QString("can not create folder(%1)").arg(saveCertFileFolder_));
			return false;
		}
	}

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
	ssize_t count = ch.extensions_.size();
	for (int i = 0; i < count; i++) {
		GTls::Extension* extension = ch.extensions_.at(i);
		uint16_t type = extension->type();
		if (type == GTls::Extension::ServerName) {
			GTls::ServerNameIndication* sni = GTls::PServerNameIndication(extension);
			QByteArray serverName = sni->serverName();
			qDebug() << QString("serverName=%1").arg(serverName);
			return serverName;
		}
	}
	qWarning() << "Can not find ServerName";
	return QString();
}

QList<QByteArray> GCertMgr::extractCertificates(GTls::Handshake *hs) {
	GTls::CertificateHs cr;
	cr.parse(hs);
	return cr.certificates_;
}

bool GCertMgr::makeFolder(QString& folder) {
	if (folder == "") return true;
	if (folder == QDir::separator()) {
		qWarning() << "folder can not be" << QDir::separator();
		return false;
	}
	if (!folder.endsWith(QDir::separator()))
		folder += QDir::separator();
	QDir dir;
	return dir.mkpath(folder);
}

void GCertMgr::saveCertFiles(QString folder, QString serverName, struct timeval ts, QList<QByteArray>& certs) {
	qint64 msecs = ts.tv_sec;
	msecs = msecs * 1000 +  ts.tv_usec / 1000;
	QDateTime now = QDateTime::fromMSecsSinceEpoch(msecs);
	for (int i = 0; i < certs.size(); i++) {
		QByteArray cert =  certs.at(i);
		QString fileName = QString("%1%2-%3-%4.der").arg(folder).arg(now.toString("yyMMdd-hhmmss-zzz")).arg(serverName).arg(i);
		QFile file(fileName);
		file.open(QIODevice::WriteOnly);
		file.write(cert);
		file.close();
	}
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

		if (size < sizeof(GTls::Record)) {
			qDebug() << QString("size(%1) is too short").arg(size); // gilgil temp 2024.10.01
			return;
		}
		GTls::Record* tr = GTls::PRecord(p);
		bool ok = true;
		uint8_t contentType = tr->contentType();
		if (contentType != GTls::Record::Handshake) { // may be not TLS handshake
			ok = false;
		} else {
			uint16_t version = tr->version();
			switch (version) {
				case GTls::Record::Tls1_0:
				case GTls::Record::Tls1_1:
				case GTls::Record::Tls1_2:
				case GTls::Record::Tls1_3:
					break;
				default:
					ok = false;
			}
		}
		if (!ok) {
			item->checkNeeded_ = false;
			return;
		}
		uint16_t r = sizeof(GTls::Record) + tr->length();
		if (size < r) {
			qDebug() << QString("size(%1) is less then r(%2)").arg(size).arg(r); // gilgil temp 2024.10.01
			return;
		}
		size -= sizeof(GTls::Record);

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
				QList<QByteArray> certs = extractCertificates(hs);
				item->checkNeeded_ = false;

				Item* revItem = getItem(tcpFlowMgr_->currentRevTcpFlowVal_);
				QString serverName;
				if (revItem != nullptr)
					serverName = revItem->serverName_;
				qDebug() << QString("cert detected %1 %2").arg(serverName).arg(certs.size()); // gilgil temp 2024.10.01
				if (saveCertFile_) {
					saveCertFiles(saveCertFileFolder_, serverName, packet->ts_, certs);
				}
				emit certificatesDetected(revItem->serverName_, certs);
				break;
			}
			default:
				break;
		}
		item->segment_.remove(0, sizeof(GTls::Record) + sizeof(GTls::Handshake) + length);
	}
}
