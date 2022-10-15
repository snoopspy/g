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
#include "net/gatm.h"

// ----------------------------------------------------------------------------
// GArpBlock
// ----------------------------------------------------------------------------
struct G_EXPORT GArpBlock : GStateObj, GHostMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(ulong infectInterval MEMBER infectInterval_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(QString fakeMac READ getFakeMac WRITE setFakeMac)
	Q_PROPERTY(Policy defaultPolicy MEMBER defaultPolicy_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)
	Q_PROPERTY(GObjPtr hostMgr READ getHostMgr WRITE setHostMgr)
	Q_ENUMS(Policy)

	QString getFakeMac() { return QString(fakeMac_); }
	void setFakeMac(QString value) { fakeMac_ = GMac(value); }
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
	GDuration infectInterval_{1000};
	GDuration sendInterval_{10};
	GMac fakeMac_{"00:11:22:33:44:55"};
	Policy defaultPolicy_{Block};
	GPcapDevice* pcapDevice_{nullptr};
	GHostMgr* hostMgr_{nullptr};

	Q_INVOKABLE GArpBlock(QObject* parent = nullptr);
	~GArpBlock() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// GHostMgr::Managable
	size_t itemOffset_{0};
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;

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
	struct ItemList : QList<PItem> {
		QMutex m_;
	} itemList_;

protected:
	GAtm atm_;
	GEthArpPacket infectPacket_;
	GEthArpPacket recoverPacket_;
	void infect(Item* item);
	void recover(Item* item);

protected:
	struct InfectThread : GThread {
		InfectThread(GArpBlock* arpBlock) : GThread(arpBlock), arpBlock_(arpBlock) {}
		void run() override;
		GArpBlock* arpBlock_;
		GWaitEvent we_;
	} infectThread_{this};
};
