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

#include "ghostmgr.h"
#include "net/capture/gpcapdevice.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GHostWatch
// ----------------------------------------------------------------------------
struct G_EXPORT GHostWatch : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(ulong checkInterval MEMBER checkInterval_)
	Q_PROPERTY(long scanTimeoutSec MEMBER scanTimeoutSec_)
	Q_PROPERTY(int sendPercentage MEMBER sendPercentage_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)
	Q_PROPERTY(GObjPtr hostMgr READ getHostMgr WRITE setHostMgr)

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }
	GObjPtr getHostMgr() { return hostMgr_; }
	void setHostMgr(GObjPtr value) { hostMgr_ = dynamic_cast<GHostMgr*>(value.data()); }

public:
	bool enabled_{true};
	GDuration checkInterval_{1000}; // 1 sec
	long  scanTimeoutSec_{50}; // 50 secs
	int sendPercentage_{90}; // 90 %
	GDuration sendInterval_{100}; // 100 msec
	GPcapDevice* pcapDevice_{nullptr};
	GHostMgr* hostMgr_{nullptr};

public:
	Q_INVOKABLE GHostWatch(QObject* parent = nullptr);
	~GHostWatch() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	bool propLoad(QJsonObject jo, QMetaProperty mpro) override;

protected:
	struct WatchThread : GThread {
		WatchThread(GHostWatch* hostWatch) : GThread(hostWatch) {}
		void run() override;
		GStateWaitEvent swe_;

		struct SendInfo {
			GMac mac_;
			GIp ip_;
			SendInfo() {}
			SendInfo(GMac mac, GIp ip) : mac_(mac), ip_(ip) {}
		};
	} watchThread_{this};

	void run();
};
