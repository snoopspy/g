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
		"	ip INTEGER,"
		"	host TEXT,"
		"	vendor TEXT,"
		"	alias TEXT"
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

	selectDeviceQuery_ = new QSqlQuery(db_);
	if (!selectDeviceQuery_->prepare("SELECT * FROM device WHERE mac = :mac")) {
		SET_ERR(GErr::Fail, selectDeviceQuery_->lastError().text());
		return false;
	}
	insertDeviceQuery_ = new QSqlQuery(db_);
	if (!insertDeviceQuery_->prepare("INSERT INTO device (mac, ip, host, vendor, alias) VALUES (:mac, :ip, :host, :vendor, :alias)")) {
		SET_ERR(GErr::Fail, insertDeviceQuery_->lastError().text());
		return false;
	}
	updateDeviceQuery_ = new QSqlQuery(db_);
	if (!updateDeviceQuery_->prepare("UPDATE device SET ip = :ip, host = :host, vendor = :vendor, alias = :alias WHERE mac = :mac")) {
		SET_ERR(GErr::Fail, updateDeviceQuery_->lastError().text());
		return false;
	}
	insertLogQuery_ = new QSqlQuery(db_);
	if (!insertLogQuery_->prepare("INSERT INTO log (mac, ip, beg_time, end_time) VALUES (:mac, :ip, :beg_time, :end_time)")) {
		SET_ERR(GErr::Fail, insertLogQuery_->lastError().text());
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
	if (updateDeviceQuery_ != nullptr) {
		delete updateDeviceQuery_ ;
		updateDeviceQuery_  = nullptr;
	}

	return true;
}

Db::Device Db::selectDevice(GMac mac) {
	Device res;
	selectDeviceQuery_->bindValue(":mac", quint64(uint64_t(mac)));
	if (!selectDeviceQuery_->exec()) {
		qDebug() << selectDeviceQuery_->lastError().text();
	}
	if (selectDeviceQuery_->next()) {
		res.mac_ = selectDeviceQuery_->value("mac").toULongLong();
		res.ip_ = selectDeviceQuery_->value("ip").toUInt();
		res.host_ = selectDeviceQuery_->value("host").toString();
		res.vendor_ = selectDeviceQuery_->value("vendor").toString();
		res.alias_ = selectDeviceQuery_->value("alias").toString();
	}
	return res;
}

bool Db::insertDevice(Db::Device device) {
	insertDeviceQuery_->bindValue(":mac", quint64(device.mac_));
	insertDeviceQuery_->bindValue(":ip", device.ip_);
	insertDeviceQuery_->bindValue(":host", device.host_);
	insertDeviceQuery_->bindValue(":vendor", device.vendor_);
	insertDeviceQuery_->bindValue(":alias", device.alias_);
	bool res = insertDeviceQuery_->exec();
	if (!res) {
		qDebug() << insertDeviceQuery_->lastError().text();
	}
	return res;
}

bool Db::updateDevice(Db::Device device) {
	updateDeviceQuery_->bindValue(":ip", device.ip_);
	updateDeviceQuery_->bindValue(":host", device.host_);
	updateDeviceQuery_->bindValue(":vendor", device.vendor_);
	updateDeviceQuery_->bindValue(":alias", device.alias_);
	updateDeviceQuery_->bindValue(":mac", quint64(device.mac_));
	bool res = updateDeviceQuery_->exec();
	if (!res) {
		qDebug() << updateDeviceQuery_->lastError().text();
	}
	return res;
}

bool Db::insertOrUpdateDevice(Device device) {
	Device dbDevice = selectDevice(device.mac_);
	if (dbDevice.isNull())
		return insertDevice(device);

	Device newDevice;
	newDevice.mac_ = device.mac_ == 0 ? dbDevice.mac_ : device.mac_;
	newDevice.ip_ = device.ip_ == 0 ? dbDevice.ip_ : device.ip_;
	newDevice.host_ = device.host_ == "" ? dbDevice.host_ : device.host_;
	newDevice.vendor_ = device.vendor_ == "" ? dbDevice.vendor_ : device.vendor_;
	newDevice.alias_ = device.alias_ == "" ? dbDevice.alias_ : device.alias_;
	return updateDevice(newDevice);
}

bool Db::insertLog(GMac mac, GIp ip, time_t begTime, time_t endTime) {
	insertLogQuery_->bindValue(":mac", quint64(uint64_t(mac)));
	insertLogQuery_->bindValue(":ip", uint32_t(ip));
	insertLogQuery_->bindValue(":beg_time", quint64(begTime));
	insertLogQuery_->bindValue(":end_time", quint64(endTime));
	bool res = insertLogQuery_->exec();
	if (!res) {
		qDebug() << insertLogQuery_->lastError().text();
	}
	return res;
}
