// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <GHostMgr>

// ----------------------------------------------------------------------------
// GHostDb
// ----------------------------------------------------------------------------
struct G_EXPORT GHostDb : GStateObj, GHostMgr::Managable, QRecursiveMutex {
	Q_OBJECT
	Q_PROPERTY(QString fileName MEMBER fileName_)
	Q_PROPERTY(GObjPtr hostMgr READ getHostMgr WRITE setHostMgr)

	GObjPtr getHostMgr() { return hostMgr_; }
	void setHostMgr(GObjPtr value) { hostMgr_ = dynamic_cast<GHostMgr*>(value.data()); }

public:
	QString fileName_{"host.db"};
	GHostMgr* hostMgr_{nullptr};

public:
	Q_INVOKABLE GHostDb(QObject* parent = nullptr);
	~GHostDb() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	enum Mode {
		Default = 0,
		Allow = 1,
		Block = 2
	};
	struct Item : GHostMgr::HostValue {
		QString alias_;
		Mode mode_{Default};
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

	// GHostMgr::Managable
	size_t itemOffset_;
	Item* getItem(GHostMgr::HostValue* hostValue) { return PItem(hostValue->mem(itemOffset_)); }
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostChanged(GMac mac, GHostMgr::HostValue* hostValue) override;

public:
	QSqlDatabase db_;

protected:
	QSqlQuery* selectHostQuery_{nullptr};
	QSqlQuery* insertHostQuery_{nullptr};
	QSqlQuery* updateHostQuery_{nullptr};
	QSqlQuery* insertLogQuery_{nullptr};

public:
	bool selectHost(GMac mac, Item* item);
	bool insertHost(GMac mac, Item* item);
	bool updateHost(GMac mac, Item* item);
	bool insertOrUpdateDevice(GMac mac, Item* item);
	bool insertLog(GMac mac, GIp ip, time_t sttTime, time_t endTime);

public:
	QString getDefaultName(GMac mac, Item* item = nullptr);
};
