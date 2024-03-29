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
#include "net/manage/ghostdb.h"
#include "net/pdu/getharppacket.h"
#include "net/write/gpcapdevicewrite.h"
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
	Q_PROPERTY(AttackDirection attackDirection MEMBER attackDirection_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)
	Q_PROPERTY(GObjPtr hostMgr READ getHostMgr WRITE setHostMgr)
	Q_PROPERTY(GObjPtr hostDb READ getHostDb WRITE setHostDb)
	Q_ENUMS(Policy)
	Q_ENUMS(AttackDirection)

	QString getFakeMac() { return QString(fakeMac_); }
	void setFakeMac(QString value) { fakeMac_ = GMac(value); }
	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }
	GObjPtr getHostMgr() { return hostMgr_; }
	void setHostMgr(GObjPtr value) { hostMgr_ = dynamic_cast<GHostMgr*>(value.data()); }
	GObjPtr getHostDb() { return hostDb_; }
	void setHostDb(GObjPtr value) { hostDb_ = dynamic_cast<GHostDb*>(value.data()); }

public:
	enum Policy {
		Allow = 1,
		Block = 2
	};

	enum AttackDirection {
		Host,
		Gateway,
		Both
	};

	bool enabled_{true};
	GDuration infectInterval_{1000};
	GDuration sendInterval_{10};
	GMac fakeMac_{"00:11:22:33:44:55"};
	Policy defaultPolicy_{Block};
	AttackDirection attackDirection_{Both};
	GPcapDevice *pcapDevice_{nullptr};
	GHostMgr *hostMgr_{nullptr};
	GHostDb* hostDb_{nullptr};
	GPcapDeviceWrite write_{this};

	Q_INVOKABLE GArpBlock(QObject *parent = nullptr);
	~GArpBlock() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		GMac mac_{GMac::nullMac()};
		GIp ip_{0};
		Policy policy_{Allow};
	};
	typedef Item *PItem;

	struct ItemMap : QMap<GMac, Item*>, QRecursiveMutex {
	} itemMap_;
	// --------------------------------------------------------------------------

	// GHostMgr::Managable
	size_t itemOffset_;
	Item* getItem(GHostMgr::HostValue* hostValue) { return PItem(hostValue->mem(itemOffset_)); }
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostChanged(GMac mac, GHostMgr::HostValue* hostValue) override;

protected:
	GAtm atm_;
	GMac mac_{GMac::nullMac()};
	GIp ip_{0};
	GMac gwMac_{GMac::nullMac()};
	GIp gwIp_{0};

	QMutex sendMutex_;
	GEthArpPacket sendPacket_;

public:
	void infect(Item* item, uint16_t operation);
	void recover(Item* item, uint16_t operation);

protected:
	struct InfectThread : GThread {
		InfectThread(GArpBlock* arpBlock) : GThread(arpBlock), arpBlock_(arpBlock) {}
		void run() override;
		GArpBlock* arpBlock_;
		GStateWaitEvent swe_;
	} infectThread_{this};
};
