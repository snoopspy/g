#include "db.h"

Db::Db(QObject* parent) : GStateObj(parent) {
}

Db::~Db() {
	close();
}

bool Db::doOpen() {
	db_ = QSqlDatabase::addDatabase("QSQLITE");
	db_.setDatabaseName(fileName_);
	if (!db_.open()) {
		SET_ERR(GErr::Fail, db_.lastError().text());
		return false;
	}

	QSqlQuery query(db_);
	if (!query.exec(
		"CREATE TABLE IF NOT EXISTS device ("
		"	mac INTEGER PRIMARY KEY,"
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
		"	beg_time INTEGER,"\
		"	end_time INTEGER"\
		");"
	)) {
		SET_ERR(GErr::Fail, query.lastError().text());
		return false;
	}

	selectDeviceQuery_ = new QSqlQuery(db_);
	if (!selectDeviceQuery_->prepare("SELECT * FROM device WHERE mac = :mac")) {
		SET_ERR(GErr::Fail, selectDeviceQuery_->lastError().text());
		return false;
	}

	insertDeviceQuery_ = new QSqlQuery(db_);
	if (!insertDeviceQuery_->prepare("INSERT INTO device(mac, alias, host, vendor) VALUES(:mac, :alias, :host, :vendor)")) {
		SET_ERR(GErr::Fail, insertDeviceQuery_->lastError().text());
		return false;
	}

	return true;
}

bool Db::doClose() {
	db_.close();

	if (selectDeviceQuery_ != nullptr) {
		delete selectDeviceQuery_ ;
		selectDeviceQuery_  = nullptr;
	}
	if (insertDeviceQuery_ != nullptr) {
		delete insertDeviceQuery_ ;
		insertDeviceQuery_  = nullptr;
	}

	return true;
}

Db::Device Db::selectDevice(GMac mac) {
	Device res;
	selectDeviceQuery_->bindValue(0, qint64(mac));
	if (!selectDeviceQuery_->exec()) {
		qDebug() << selectDeviceQuery_->lastError().text();
	}
	if (selectDeviceQuery_->next()) {
		res.mac_ = selectDeviceQuery_->value("mac").toLongLong();
		res.alias_ = selectDeviceQuery_->value("alias").toString();
		res.host_ = selectDeviceQuery_->value("host").toString();
		res.vendor_ = selectDeviceQuery_->value("vendor").toString();
	}
	return res;
}

bool Db::insertDevice(Db::Device device) {
	insertDeviceQuery_->bindValue("mac", device.mac_);
	insertDeviceQuery_->bindValue("alias", device.alias_);
	insertDeviceQuery_->bindValue("host", device.host_);
	insertDeviceQuery_->bindValue("vendor", device.vendor_);
	bool res = insertDeviceQuery_->exec();
	if (!res) {
		qDebug() << selectDeviceQuery_->lastError().text();
	}
	return res;
}
