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
// HostDb
// ----------------------------------------------------------------------------
struct G_EXPORT HostDb : GStateObj, GHostMgr::Managable, QRecursiveMutex {
	Q_OBJECT
	Q_PROPERTY(QString fileName MEMBER fileName_)
	Q_PROPERTY(GObjPtr hostMgr READ getHostMgr WRITE setHostMgr)

	GObjPtr getHostMgr() { return hostMgr_; }
	void setHostMgr(GObjPtr value) { hostMgr_ = dynamic_cast<GHostMgr*>(value.data()); }

public:
	QString fileName_{"host.db"};
	GHostMgr* hostMgr_{nullptr};

public:
	Q_INVOKABLE HostDb(QObject* parent = nullptr);
	~HostDb() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// GHostMgr::Managable
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostChanged(GMac mac, GHostMgr::HostValue* hostValue) override;

public:
	QSqlDatabase db_;

protected:
	QSqlQuery* selectHostQuery_{nullptr};
	QSqlQuery* insertHostQuery_{nullptr};
	QSqlQuery* updateHostQuery_{nullptr};
	QSqlQuery* updateAliasQuery_{nullptr};
	QSqlQuery* insertLogQuery_{nullptr};

public:
	bool selectHost(GMac mac, GHostMgr::HostValue* hostValue);
	bool insertHost(GMac mac, GHostMgr::HostValue* hostValue);
	bool updateHost(GMac mac, GHostMgr::HostValue* hostValue);
	bool updateAlias(GMac mac, QString alias);
	bool insertOrUpdateDevice(GMac mac, GHostMgr::HostValue* hostValue);
	bool insertLog(GMac mac, GIp ip, time_t begTime, time_t endTime);

public:
	QString getDefaultName(GMac mac, GHostMgr::HostValue* hostValue = nullptr);
};
