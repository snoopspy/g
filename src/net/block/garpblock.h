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

#include "net/manage/ghostmgr.h"
#include "net/pdu/getharppacket.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GArpBlock
// ----------------------------------------------------------------------------
struct G_EXPORT GArpBlock : GStateObj, GHostMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(ulong infectInterval MEMBER infectInterval_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(Policy defaultPolicy MEMBER defaultPolicy_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)
	Q_PROPERTY(GObjPtr hostMgr READ getHostMgr WRITE setHostMgr)
	Q_ENUMS(Policy)

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }
	GObjPtr getHostMgr() { return hostMgr_; }
	void setHostMgr(GObjPtr value) { hostMgr_ = dynamic_cast<GHostMgr*>(value.data()); }

public:
	enum Policy {
		Allow,
		Block
	};

	bool enabled_{true};
	GPcapDevice* pcapDevice_{nullptr};
	GHostMgr* hostMgr_{nullptr};
	GDuration infectInterval_{1000};
	GDuration sendInterval_{10};
	Policy defaultPolicy_{Block};

	Q_INVOKABLE GArpBlock(QObject* parent = nullptr);
	~GArpBlock() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// GHostMgr::Managable
	size_t itemOffset_{0};
	void hostDetected(GMac mac, GHostMgr::Value* value) override;
	void hostDeleted(GMac mac, GHostMgr::Value* value) override;

	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		Item(Policy policy, GMac mac, GIp ip) : policy_(policy), mac_(mac), ip_(ip) {}
		Policy policy_;
		GMac mac_;
		GIp ip_;
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

protected:
	GEthArpPacket infectPacket_;

protected:
	struct InfectThread : GThread {
		InfectThread(GArpBlock* arpBlock) : GThread(arpBlock), arpBlock_(arpBlock) {}
		void run() override;
		GArpBlock* arpBlock_;
		GWaitEvent we_;
	} infectThread_{this};

public slots:
	void block(GPacket* packet);
};
