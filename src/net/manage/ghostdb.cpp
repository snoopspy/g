#include "ghostdb.h"

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
			"	vendor TEXT"
			");"
	)) {
		SET_ERR(GErr::Fail, query.lastError().text());
		return false;
	}

	if (!query.exec(
			"CREATE TABLE IF NOT EXISTS log ("\
			"	mac INTEGER,"\
			"	ip INTEGER,"\
			"	beg_time INTEGER,"\
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
	if (!insertHostQuery_->prepare("INSERT INTO host (mac, ip, host, vendor) VALUES (:mac, :ip, :host, :vendor)")) {
		SET_ERR(GErr::Fail, insertHostQuery_->lastError().text());
		return false;
	}
	updateHostQuery_ = new QSqlQuery(db_);
	if (!updateHostQuery_->prepare("UPDATE host SET ip = :ip, host = :host, vendor = :vendor WHERE mac = :mac")) {
		SET_ERR(GErr::Fail, updateHostQuery_->lastError().text());
		return false;
	}
	updateAliasQuery_ = new QSqlQuery(db_);
	if (!updateAliasQuery_->prepare("UPDATE host SET alias = :alias WHERE mac = :mac")) {
		SET_ERR(GErr::Fail, updateAliasQuery_->lastError().text());
		return false;
	}
	insertLogQuery_ = new QSqlQuery(db_);
	if (!insertLogQuery_->prepare("INSERT INTO log (mac, ip, beg_time, end_time) VALUES (:mac, :ip, :beg_time, :end_time)")) {
		SET_ERR(GErr::Fail, insertLogQuery_->lastError().text());
		return false;
	}

	return true;
}

bool GHostDb::doClose() {
	QMutexLocker(this);
	db_.close();

	if (selectHostQuery_ != nullptr) {
		delete selectHostQuery_ ;
		selectHostQuery_  = nullptr;
	}
	if (insertHostQuery_ != nullptr) {
		delete insertHostQuery_ ;
		insertHostQuery_  = nullptr;
	}
	if (updateHostQuery_ != nullptr) {
		delete updateHostQuery_ ;
		updateHostQuery_  = nullptr;
	}
	if (insertLogQuery_ != nullptr) {
		delete insertLogQuery_ ;
		insertLogQuery_  = nullptr;
	}

	return true;
}

void GHostDb::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	insertOrUpdateDevice(mac, hostValue);
}

void GHostDb::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	insertLog(mac, hostValue->ip_, hostValue->firstTs_.tv_sec, hostValue->lastTs_.tv_sec);
}

void GHostDb::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	insertOrUpdateDevice(mac, hostValue);
}

bool GHostDb::selectHost(GMac mac, GHostMgr::HostValue* hostValue) {
	QMutexLocker(this);

	selectHostQuery_->bindValue(":mac", quint64(mac));
	if (!selectHostQuery_->exec()) {
		qWarning() << selectHostQuery_->lastError().text();
		return false;
	}

	if (selectHostQuery_->next()) {
		Q_ASSERT(quint64(mac) == selectHostQuery_->value("mac").toULongLong());
		hostValue->ip_ = selectHostQuery_->value("ip").toUInt();
		hostValue->host_ = selectHostQuery_->value("host").toString();
		hostValue->vendor_ = selectHostQuery_->value("vendor").toString();
		return true;
	}
	return false;
}

bool GHostDb::insertHost(GMac mac, GHostMgr::HostValue* hostValue) {
	QMutexLocker(this);

	insertHostQuery_->bindValue(":mac", quint64(mac));
	insertHostQuery_->bindValue(":ip", uint32_t(hostValue->ip_));
	insertHostQuery_->bindValue(":host", hostValue->host_);
	insertHostQuery_->bindValue(":vendor", hostValue->vendor_);
	bool res = insertHostQuery_->exec();
	if (!res) {
		qWarning() << insertHostQuery_->lastError().text();
	}
	return res;
}

bool GHostDb::updateHost(GMac mac, GHostMgr::HostValue* hostValue) {
	QMutexLocker(this);

	updateHostQuery_->bindValue(":ip", uint32_t(hostValue->ip_));
	updateHostQuery_->bindValue(":host", hostValue->host_);
	updateHostQuery_->bindValue(":vendor", hostValue->vendor_);
	updateHostQuery_->bindValue(":mac", quint64(mac));
	bool res = updateHostQuery_->exec();
	if (!res) {
		qWarning() << updateHostQuery_->lastError().text();
	}
	return res;
}

bool GHostDb::updateAlias(GMac mac, QString alias) {
	QMutexLocker(this);

	updateAliasQuery_->bindValue(":alias", alias);
	updateAliasQuery_->bindValue(":mac", quint64(mac));
	bool res = updateAliasQuery_->exec();
	if (!res) {
		qWarning() << updateAliasQuery_->lastError().text();
	}
	return res;
}

bool GHostDb::insertOrUpdateDevice(GMac mac, GHostMgr::HostValue* hostValue) {
	QMutexLocker(this);

	GHostMgr::HostValue dbHostValue;
	if (selectHost(mac, &dbHostValue)) {
		GHostMgr::HostValue newHostValue;
		newHostValue.ip_ = hostValue->ip_ == 0 ? dbHostValue.ip_ : hostValue->ip_;
		newHostValue.host_ = hostValue->host_ == "" ? dbHostValue.host_ : hostValue->host_;
		newHostValue.vendor_ = hostValue->vendor_ == "" ? dbHostValue.vendor_ : hostValue->vendor_;
		return updateHost(mac, &newHostValue);
	}
	return insertHost(mac, hostValue);
}

bool GHostDb::insertLog(GMac mac, GIp ip, time_t begTime, time_t endTime) {
	QMutexLocker(this);

	insertLogQuery_->bindValue(":mac", quint64(mac));
	insertLogQuery_->bindValue(":ip", uint32_t(ip));
	insertLogQuery_->bindValue(":beg_time", quint64(begTime));
	insertLogQuery_->bindValue(":end_time", quint64(endTime));
	bool res = insertLogQuery_->exec();
	if (!res) {
		qWarning() << insertLogQuery_->lastError().text();
	}
	return res;
}

QString GHostDb::getDefaultName(GMac mac, GHostMgr::HostValue* hostValue) {
	QMutexLocker(this);

	QString res;

	if (hostValue != nullptr) {
		if (hostValue->host_ != "") res = hostValue->host_;
		else if (hostValue->vendor_ != "") res = hostValue->vendor_;
	}

	selectHostQuery_->bindValue(":mac", quint64(mac));
	if (!selectHostQuery_->exec()) {
		qWarning() << selectHostQuery_->lastError().text();
	} else if (selectHostQuery_->next()) {
		QString alias = selectHostQuery_->value("alias").toString();
		if (alias != "") return alias;
		if (res == "") {
			QString host = selectHostQuery_->value("host").toString();
			if (host != "") return host;
			QString vendor = selectHostQuery_->value("vendor").toString();
			if (vendor != "") return host;
		}
	}

	if (res == "") res = QString(mac);
	return res;
}
