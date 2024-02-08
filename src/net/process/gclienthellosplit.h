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

#include "base/gstateobj.h"
#include "net/manage/gtcpflowmgr.h"
#include "net/write/grawipsocketwrite.h"

// ----------------------------------------------------------------------------
// GClientHelloSplit
// ----------------------------------------------------------------------------
struct G_EXPORT GClientHelloSplit : GStateObj, GTcpFlowMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(int bufSize MEMBER bufSize_)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)
	Q_PROPERTY(GObjRef writer READ getWriter)

	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }
	GObjRef getWriter() { return &writer_; }

public:
	int bufSize_{GPacket::MaxBufSize};
	GTcpFlowMgr* tcpFlowMgr_{nullptr};
	GRawIpSocketWrite writer_{this};

public:
	Q_INVOKABLE GClientHelloSplit(QObject* parent = nullptr) : GStateObj(parent) {}
	~GClientHelloSplit() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	gbyte* splittedTcpDataBuf_{nullptr};

public:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		bool processed_{false};
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

	// GTcpFlowMgr::Managable
	size_t tcpFlowOffset_{0};
	Item* getItem(GTcpFlowMgr::TcpFlowValue* tcpFlowValue) { return PItem(tcpFlowValue->mem(tcpFlowOffset_)); }
	void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;

public slots:
	void split(GPacket* packet);
};
