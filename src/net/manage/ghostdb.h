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

#include "ghostmgr.h"

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
		Auto = 0,
		Allow = 1,
		Block = 2
	};
	struct Item {
		GMac mac_{GMac::nullMac()};
		GIp ip_{0};
		QString alias_;
		QString host_;
		QString vendor_;
		Mode mode_{Auto};

		QString getDefaultName() {
			if (alias_ != "") return alias_;
			if (host_ != "") return host_;
			if (vendor_ != "") return vendor_;
			return QString(mac_);
		}
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

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
	QSqlQuery* insertLogQuery_{nullptr};

public:
	bool selectHost(GMac mac, Item* item);
	bool insertHost(GMac mac, Item* item);
	bool updateHost(GMac mac, Item* item);
	bool insertOrUpdateDevice(GMac mac, Item* item);
	bool insertLog(GMac mac, GIp ip, time_t sttTime, time_t endTime);

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
