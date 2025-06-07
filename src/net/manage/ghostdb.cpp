#include "ghostdb.h"

#ifdef QT_SQL_LIB

// ----------------------------------------------------------------------------
// GHostDb
// ----------------------------------------------------------------------------
GHostDb::GHostDb(QObject* parent) : GStateObj(parent) {
}

GHostDb::~GHostDb() {
	close();
}

bool GHostDb::doOpen() {
	if (hostMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "hostMgr is null");
		return false;
	}
	hostMgr_->managables_.insert(this);

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
			"CREATE TABLE IF NOT EXISTS host ("
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

	selectHostQuery_ = new QSqlQuery(db_);
	if (!selectHostQuery_->prepare("SELECT * FROM host WHERE mac = :mac")) {
		SET_ERR(GErr::Fail, selectHostQuery_->lastError().text());
		return false;
	}
	insertHostQuery_ = new QSqlQuery(db_);
	if (!insertHostQuery_->prepare("INSERT INTO host (mac, ip, alias, host, vendor, mode) VALUES (:mac, :ip, :alias, :host, :vendor, :mode)")) {
		SET_ERR(GErr::Fail, insertHostQuery_->lastError().text());
		return false;
	}
	updateHostQuery_ = new QSqlQuery(db_);
	if (!updateHostQuery_->prepare("UPDATE host SET ip = :ip, alias = :alias, host = :host, vendor = :vendor, mode = :mode WHERE mac = :mac")) {
		SET_ERR(GErr::Fail, updateHostQuery_->lastError().text());
		return false;
	}
	insertLogQuery_ = new QSqlQuery(db_);
	if (!insertLogQuery_->prepare("INSERT INTO log (mac, ip, stt_time, end_time) VALUES (:mac, :ip, :stt_time, :end_time)")) {
		SET_ERR(GErr::Fail, insertLogQuery_->lastError().text());
		return false;
	}

	return true;
}

bool GHostDb::doClose() {
	QMutexLocker ml(this);
	db_.close();

	if (selectHostQuery_ != nullptr) {
		delete selectHostQuery_;
		selectHostQuery_ = nullptr;
	}
	if (insertHostQuery_ != nullptr) {
		delete insertHostQuery_;
		insertHostQuery_ = nullptr;
	}
	if (updateHostQuery_ != nullptr) {
		delete updateHostQuery_;
		updateHostQuery_ = nullptr;
	}
	if (insertLogQuery_ != nullptr) {
		delete insertLogQuery_;
		insertLogQuery_ = nullptr;
	}

	return true;
}

void GHostDb::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	Item item;
	item.mac_ = mac;
	item.ip_ = hostValue->ip_;
	item.host_ = hostValue->host_;
	item.vendor_ = hostValue->vendor_;
	insertOrUpdateDevice(mac, &item);
}

void GHostDb::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	insertLog(mac, hostValue->ip_, hostValue->firstTime_.tv_sec, hostValue->lastTime_.tv_sec);
}

void GHostDb::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	Item item;
	item.mac_ = mac;
	item.ip_ = hostValue->ip_;
	item.host_ = hostValue->host_;
	item.vendor_ = hostValue->vendor_;
	insertOrUpdateDevice(mac, &item);
}

bool GHostDb::selectHost(GMac mac, Item* item) {
	QMutexLocker ml(this);

	Q_ASSERT(selectHostQuery_ != nullptr);
	selectHostQuery_->bindValue(":mac", quint64(mac));
	if (!selectHostQuery_->exec()) {
		qWarning() << selectHostQuery_->lastError().text();
		return false;
	}

	if (selectHostQuery_->next()) {
		Q_ASSERT(quint64(mac) == selectHostQuery_->value("mac").toULongLong());
		item->mac_ = mac;
		item->ip_ = selectHostQuery_->value("ip").toUInt();
		item->alias_ = selectHostQuery_->value("alias").toString();
		item->host_ = selectHostQuery_->value("host").toString();
		item->vendor_ = selectHostQuery_->value("vendor").toString();
		int i = Mode(selectHostQuery_->value("mode").toInt());
		item->mode_ = Mode(i);
		return true;
	}
	return false;
}

bool GHostDb::insertHost(GMac mac, Item* item) {
	QMutexLocker ml(this);

	Q_ASSERT(insertHostQuery_ != nullptr);
	insertHostQuery_->bindValue(":mac", quint64(mac));
	insertHostQuery_->bindValue(":ip", uint32_t(item->ip_));
	insertHostQuery_->bindValue(":alias", item->alias_);
	insertHostQuery_->bindValue(":host", item->host_);
	insertHostQuery_->bindValue(":vendor", item->vendor_);
	insertHostQuery_->bindValue(":mode", int(item->mode_));
	bool res = insertHostQuery_->exec();
	if (!res) {
		qWarning() << insertHostQuery_->lastError().text();
	}
	return res;
}

bool GHostDb::updateHost(GMac mac, Item *item) {
	QMutexLocker ml(this);

	Q_ASSERT(updateHostQuery_ != nullptr);
	updateHostQuery_->bindValue(":ip", uint32_t(item->ip_));
	updateHostQuery_->bindValue(":alias", item->alias_);
	updateHostQuery_->bindValue(":host", item->host_);
	updateHostQuery_->bindValue(":vendor", item->vendor_);
	updateHostQuery_->bindValue(":mode", int(item->mode_));
	updateHostQuery_->bindValue(":mac", quint64(mac));
	bool res = updateHostQuery_->exec();
	if (!res) {
		qWarning() << updateHostQuery_->lastError().text();
	}
	return res;
}

bool GHostDb::insertOrUpdateDevice(GMac mac, Item* item) {
	QMutexLocker ml(this);

	Item dbItem;
	if (selectHost(mac, &dbItem)) {
		if (item->ip_ == 0) item->ip_ = dbItem.ip_;
		if (item->alias_ == "") item->alias_ = dbItem.alias_;
		if (item->host_ == "") item->host_ = dbItem.host_;
		if (item->vendor_ == "") item->vendor_ = dbItem.vendor_;
		item->mode_ = dbItem.mode_;
		return updateHost(mac, item);
	}
	return insertHost(mac, item);
}

bool GHostDb::insertLog(GMac mac, GIp ip, time_t sttTime, time_t endTime) {
	QMutexLocker ml(this);

	Q_ASSERT(insertLogQuery_ != nullptr);
	insertLogQuery_->bindValue(":mac", quint64(mac));
	insertLogQuery_->bindValue(":ip", quint32(ip));
	insertLogQuery_->bindValue(":stt_time", quint64(sttTime));
	insertLogQuery_->bindValue(":end_time", quint64(endTime));
	bool res = insertLogQuery_->exec();
	if (!res) {
		qWarning() << insertLogQuery_->lastError().text();
	}
	return res;
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-filepath.h"
GPropItem* GHostDb::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "fileName") {
		GPropItemFilePath* res = new GPropItemFilePath(param);
		QStringList filters{"db files - *.db(*.db)", "any files - *(*)"};
		res->fd_->setNameFilters(filters);
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB

#endif // QT_SQL_LIB
