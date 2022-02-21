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
#include "net/manage/ghostmgr.h"
#include "net/manage/gipflowmgr.h"
#include "net/manage/gtcpflowmgr.h"
#include "net/manage/gudpflowmgr.h"

// ----------------------------------------------------------------------------
// GPacketMgrDebug
// ----------------------------------------------------------------------------
struct G_EXPORT GPacketMgrDebug : GStateObj, GHostMgr::Managable, GIpFlowMgr::Managable, GTcpFlowMgr::Managable, GUdpFlowMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(GObjPtr hostMgr READ getHostMgr WRITE setHostMgr)
	Q_PROPERTY(GObjPtr ipFlowMgr READ getIpFlowMgr WRITE setIpFlowMgr)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)
	Q_PROPERTY(GObjPtr udpFlowMgr READ getUdpFlowMgr WRITE setUdpFlowMgr)

public:
	GObjPtr getHostMgr() { return hostMgr_; }
	GObjPtr getIpFlowMgr() { return ipFlowMgr_; }
	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	GObjPtr getUdpFlowMgr() { return udpFlowMgr_; }

	void setHostMgr(GObjPtr value) { hostMgr_ = dynamic_cast<GHostMgr*>(value.data()); }
	void setIpFlowMgr(GObjPtr value) { ipFlowMgr_ = dynamic_cast<GIpFlowMgr*>(value.data()); }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }
	void setUdpFlowMgr(GObjPtr value) { udpFlowMgr_ = dynamic_cast<GUdpFlowMgr*>(value.data()); }

public:
	bool enabled_{true};
	GHostMgr* hostMgr_{nullptr};
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
		FlowItem() {
			GDEBUG_CTOR
		}
		~FlowItem() {
			GDEBUG_DTOR
		}
	};
	typedef FlowItem *PFlowItem;
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GPacketMgrDebug(QObject* parent = nullptr) : GStateObj(parent) {}
	~GPacketMgrDebug() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	size_t hostOffset_{0};
	size_t ipFlowOffset_{0};
	size_t tcpFlowOffset_{0};
	size_t udpFlowOffset_{0};

public:
	// GHostMgr::Managable
	void hostDetected(GMac mac, GPacketMgr::Value* value) override;
	void hostDeleted(GMac mac, GPacketMgr::Value* value) override;

	// GIpFlowMgr::Managable
	void ipFlowDetected(GFlow::IpFlowKey ipFlowKey, GPacketMgr::Value* value) override;
	void ipFlowDeleted(GFlow::IpFlowKey ipFlowKey, GPacketMgr::Value* value) override;

	// GTcpFlowMgr::Managable
	void tcpFlowDetected(GFlow::TcpFlowKey tcpFlowKey, GPacketMgr::Value* value) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GPacketMgr::Value* value) override;

	// GUdpFlowMgr::Managable
	void udpFlowDetected(GFlow::UdpFlowKey udpFlowKey, GPacketMgr::Value* value) override;
	void udpFlowDeleted(GFlow::UdpFlowKey udpFlowKey, GPacketMgr::Value* value) override;

public slots:
	void debug(GPacket* packet);

signals:
	void debugged(GPacket* packet);
};
