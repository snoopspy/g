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

	return true;
}

bool GCookieHijack::doClose() {
	return true;
}

void GCookieHijack::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	// qDebug() << QString("_tcpFlowDetected %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_), QString::number(tcpFlowKey.sport_), QString(tcpFlowKey.dip_), QString::number(tcpFlowKey.dport_)); // gilgil temp 2021.04.07
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	new (item) Item;
}

void GCookieHijack::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	item->~Item();
	// qDebug() << QString("_tcpFlowDeleted %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_), QString::number(tcpFlowKey.sport_), QString(tcpFlowKey.dip_), QString::number(tcpFlowKey.dport_)); // gilgil temp 2021.04.07
}

void GCookieHijack::hijack(GPacket* packet) {
	qDebug() << packet->buf_.size_; // gilgil temp 2024.02.22

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;
	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) return;

	GBuf tcpData = packet->tcpData_;
	if (!tcpData.valid()) return;

	QString segment = QString::fromLatin1(pchar(tcpData.data_), tcpData.size_);

	Q_ASSERT(tcpFlowMgr_->currentTcpFlowVal_ != nullptr);
	Item* item = getItem(tcpFlowMgr_->currentTcpFlowVal_);

	item->insertSegment(tcpHdr->seq(), segment, maxMergeCount_);

	QString host;
	CookieList cookies;
	if (!item->extract(host, cookies, reFindCookie_))
		return;

	QString msg = QString("find cookies for : %1").arg(host);
	for (Cookie& cookie: cookies) {
		msg += cookie.name_ + ":" + cookie.value_ + " ";
	}
	qDebug() << msg;

	emit hijacked(packet, &host, &cookies);
}

// ----------------------------------------------------------------------------
// GCookieHijack::Item
// ----------------------------------------------------------------------------
void GCookieHijack::Item::insertSegment(uint32_t seq, QString segment, int maxMergeCount) {
	segments_.insert(seq, segment);
	while (segments_.count() > maxMergeCount) {
		segments_.erase(segments_.begin());
	}
	for (Map::iterator it = segments_.begin(); it != segments_.end(); it++) {
		qDebug() << it.key() << it.value();
	}
}

bool GCookieHijack::Item::extract(QString& host, CookieList& cookies, QRegularExpression& findCookieRe_) {
	return false;
}
