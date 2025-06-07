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

#ifdef QT_SQL_LIB

#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "base/gstateobj.h"
#include "net/manage/gtcpflowmgr.h"
#include "net/pdu/gtcpsegment.h"

// ----------------------------------------------------------------------------
// GCookieHijack
// ----------------------------------------------------------------------------
struct G_EXPORT GCookieHijack : GStateObj, GTcpFlowMgr::Managable, QRecursiveMutex {
	Q_OBJECT
	Q_PROPERTY(QString fileName MEMBER fileName_)
	Q_PROPERTY(int maxMergeCount MEMBER maxMergeCount_)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)

	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }

public:
	QString fileName_{"cookiehijack.db"};
	int maxMergeCount_{5};
	GTcpFlowMgr* tcpFlowMgr_{nullptr};

public:
	Q_INVOKABLE GCookieHijack(QObject* parent = nullptr) : GStateObj(parent) {}
	~GCookieHijack() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		GTcpSegment segment_;
		Item(uint32_t seq) : segment_(seq) {}
		~Item() {}
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

	// GTcpFlowMgr::Managable
	size_t tcpFlowOffset_{0};
	Item* getItem(GTcpFlowMgr::TcpFlowValue* tcpFlowValue) { return PItem(tcpFlowValue->mem(tcpFlowOffset_)); }
	void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;

protected:
	QRegularExpression reHost_;
	QRegularExpression reCookie_;

public:
	QSqlDatabase db_;
	QSqlQuery* insertQuery_{nullptr};

public:
	bool extract(QString httpRequest, QString& host, QString& cookie);
	bool insert(time_t created, GMac mac, GIp ip, QString host, QString cookie);

public slots:
	void hijack(GPacket* packet);

signals:
	void hijacked(time_t created, GMac mac, GIp ip, QString host, QString cookie);

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};

#endif // QT_SQL_LIB
