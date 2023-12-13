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
	Q_INVOKABLE GPacketMgrDebug(QObject* parent = nullptr) : GStateObj(parent) {}
	~GPacketMgrDebug() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		size_t packets{0};
		size_t bytes{0};
		Item() {
			GDEBUG_CTOR
		}
		~Item() {
			GDEBUG_DTOR
		}
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

public:
	// GHostMgr::Managable
	size_t hostOffset_{0};
	Item* getItem(GHostMgr::HostValue* hostValue) { return PItem(hostValue->mem(hostOffset_)); }
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostChanged(GMac mac, GHostMgr::HostValue* hostValue) override;

	// GIpFlowMgr::Managable
	size_t ipFlowOffset_{0};
	Item* getItem(GIpFlowMgr::IpFlowValue* ipFlowValue) { return PItem(ipFlowValue->mem(ipFlowOffset_)); }
	void ipFlowCreated(GFlow::IpFlowKey ipFlowKey, GIpFlowMgr::IpFlowValue* ipFlowValue) override;
	void ipFlowDeleted(GFlow::IpFlowKey ipFlowKey, GIpFlowMgr::IpFlowValue* ipFlowValue) override;

	// GTcpFlowMgr::Managable
	size_t tcpFlowOffset_{0};
	Item* getItem(GTcpFlowMgr::TcpFlowValue* tcpFlowValue) { return PItem(tcpFlowValue->mem(tcpFlowOffset_)); }
	void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;

	// GUdpFlowMgr::Managable
	size_t udpFlowOffset_{0};
	Item* getItem(GUdpFlowMgr::UdpFlowValue* udpFlowValue) { return PItem(udpFlowValue->mem(udpFlowOffset_)); }
	void udpFlowCreated(GFlow::UdpFlowKey udpFlowKey, GUdpFlowMgr::UdpFlowValue* udpFlowValue) override;
	void udpFlowDeleted(GFlow::UdpFlowKey udpFlowKey, GUdpFlowMgr::UdpFlowValue* udpFlowValue) override;

public slots:
	void debugHost(GPacket* packet);
	void debugIp(GPacket* packet);
	void debugTcp(GPacket* packet);
	void debugUdp(GPacket* packet);

signals:
	void debugged(GPacket* packet);
};
