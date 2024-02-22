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

#include <QRegularExpression>

#include "base/gstateobj.h"
#include "net/manage/gtcpflowmgr.h"

// ----------------------------------------------------------------------------
// GCookieHijack
// ----------------------------------------------------------------------------
struct G_EXPORT GCookieHijack : GStateObj, GTcpFlowMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(int maxMergeCount MEMBER maxMergeCount_)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)

	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }

public:
	int maxMergeCount_{3};
	GTcpFlowMgr* tcpFlowMgr_{nullptr};

public:
	Q_INVOKABLE GCookieHijack(QObject* parent = nullptr) : GStateObj(parent) {}
	~GCookieHijack() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	QRegularExpression reHost_;
	QRegularExpression reCookie_;

public:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		Item(GCookieHijack* ch) : ch_(ch) {}
		~Item() {}
		GCookieHijack* ch_;
		struct Map : QMap<uint32_t /*seq*/, QString /*segment*/> {
		} segments_;

		void insertSegment(uint32_t seq, QString segment);
		bool extract(QString& host, QString &cookie);
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

	// GTcpFlowMgr::Managable
	size_t tcpFlowOffset_{0};
	Item* getItem(GTcpFlowMgr::TcpFlowValue* tcpFlowValue) { return PItem(tcpFlowValue->mem(tcpFlowOffset_)); }
	void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;

public slots:
	void hijack(GPacket* packet);

signals:
	void hijacked(GPacket* packet, QString host, QString cookie);
};
