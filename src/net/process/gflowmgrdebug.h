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
#include "net/flow/gipflowmgr.h"
#include "net/flow/gtcpflowmgr.h"
#include "net/flow/gudpflowmgr.h"

// ----------------------------------------------------------------------------
// GFlowMgrDebug
// ----------------------------------------------------------------------------
struct G_EXPORT GFlowMgrDebug : GStateObj, GIpFlowMgr::Managable, GTcpFlowMgr::Managable, GUdpFlowMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(GObjPtr ipFlowMgr READ getIpFlowMgr WRITE setIpFlowMgr)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)
	Q_PROPERTY(GObjPtr udpFlowMgr READ getUdpFlowMgr WRITE setUdpFlowMgr)

public:
	GObjPtr getIpFlowMgr() { return ipFlowMgr_; }
	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	GObjPtr getUdpFlowMgr() { return udpFlowMgr_; }

	void setIpFlowMgr(GObjPtr value) { ipFlowMgr_ = dynamic_cast<GIpFlowMgr*>(value.data()); }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }
	void setUdpFlowMgr(GObjPtr value) { udpFlowMgr_ = dynamic_cast<GUdpFlowMgr*>(value.data()); }

public:
	bool enabled_{true};
	GIpFlowMgr* ipFlowMgr_{nullptr};
	GTcpFlowMgr* tcpFlowMgr_{nullptr};
	GUdpFlowMgr* udpFlowMgr_{nullptr};

public:
	// --------------------------------------------------------------------------
	// FlowItem
	// --------------------------------------------------------------------------
	struct FlowItem {
		size_t packets{0};
		size_t bytes{0};
	};
	typedef FlowItem *PFlowItem;
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GFlowMgrDebug(QObject* parent = nullptr) : GStateObj(parent) {}
	~GFlowMgrDebug() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	size_t ipFlowOffset_{0};
	size_t tcpFlowOffset_{0};
	size_t udpFlowOffset_{0};

public:
	// GIpFlowMgr::Managable
	void ipFlowCreated(GFlow::IpFlowKey* key, GFlow::Value* value) override;
	void ipFlowDeleted(GFlow::IpFlowKey* key, GFlow::Value* value) override;

	// GTcpFlowMgr::Managable
	void tcpFlowCreated(GFlow::TcpFlowKey* key, GFlow::Value* value) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey* key, GFlow::Value* value) override;

	// GUdpFlowMgr::Managable
	void udpFlowCreated(GFlow::UdpFlowKey* key, GFlow::Value* value) override;
	void udpFlowDeleted(GFlow::UdpFlowKey* key, GFlow::Value* value) override;

public slots:
	void debug(GPacket* packet);

signals:
	void debugged(GPacket* packet);
};
