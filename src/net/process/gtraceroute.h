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

#include "net/manage/gipmgr.h"
#include "net/capture/gpcapdevice.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GTraceRoute
// ----------------------------------------------------------------------------
struct G_EXPORT GTraceRoute : GStateObj, GIpMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(GObjPtr ipMgr READ getIpMgr WRITE setIpMgr)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)

	GObjPtr getIpMgr() { return ipMgr_; }
	void setIpMgr(GObjPtr value) { ipMgr_ = dynamic_cast<GIpMgr*>(value.data()); }
	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }

public:
	GIpMgr* ipMgr_{nullptr};
	GPcapDevice* pcapDevice_{nullptr};

public:
	Q_INVOKABLE GTraceRoute(QObject* parent = nullptr);
	~GTraceRoute() override;

protected:
	bool doOpen() override;
	bool doClose() override;

	GIntf* intf_{nullptr};
	GIp myIp_{0};

public:
	// Managable
	size_t ipOffset_{0};
	void ipCreated(GIp ip, GIpMgr::IpValue* ipValue) override;
	void ipDeleted(GIp ip, GIpMgr::IpValue* ipValue) override;

	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		bool active_{false};
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

public slots:
	void test(GPacket* packet);

protected:
	struct TestThread : GThread {
		TestThread(GTraceRoute* tr, GPacket* packet, GIp ip);
		~TestThread() override;
		void run() override;

		GTraceRoute* tr_;
		GIp ip_;
		GAnyPacket anyPacket_;
		GPacket* packet_;
		QByteArray packetByteArray_;
		GStateWaitEvent swe_;
	};
	struct TestThreadMap : QMap<GIp, TestThread*>, QRecursiveMutex {
	} testThreadMap_;
};
