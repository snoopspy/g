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
#include "net/write/gwrite.h"

// ----------------------------------------------------------------------------
// GClientHelloSplit
// ----------------------------------------------------------------------------
struct G_EXPORT GClientHelloSplit : GStateObj, GTcpFlowMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(int bufSize MEMBER bufSize_)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)

	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }

public:
	GTcpFlowMgr* tcpFlowMgr_{nullptr};
	int bufSize_{GPacket::MaxBufSize};

	// --------------------------------------------------------------------------
	// FlowItem
	// --------------------------------------------------------------------------
	struct FlowItem {
		bool processed_{false};
	};
	typedef FlowItem *PFlowItem;
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GClientHelloSplit(QObject* parent = nullptr) : GStateObj(parent) {}
	~GClientHelloSplit() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	size_t tcpFlowOffset_{0};
	gbyte* splittedTcpDataBuf_{nullptr};

public:
	// GTcpFlowMgr::Managable
	void tcpFlowCreated(GFlow::TcpFlowKey* key, GPacketMgr::Value* value) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey* key, GPacketMgr::Value* value) override;

public slots:
	void split(GPacket* packet);

signals:
	void writeNeeded(GPacket* packet);
};
