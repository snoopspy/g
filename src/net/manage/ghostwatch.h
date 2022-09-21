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
struct G_EXPORT GHostWatch : GStateObj, GHostMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(ulong checkInterval MEMBER checkInterval_)
	Q_PROPERTY(qint64 scanStartTimeoutSec MEMBER scanStartTimeoutSec_)
	Q_PROPERTY(ulong randomInterval MEMBER randomInterval_)
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
	qint64 scanStartTimeoutSec_{50}; // 50 secs
	GDuration randomInterval_{5000}; // 5 secs
	GDuration sendInterval_{1000}; // 1 sec
	GPcapDevice* pcapDevice_{nullptr};
	GHostMgr* hostMgr_{nullptr};

public:
	Q_INVOKABLE GHostWatch(QObject* parent = nullptr);
	~GHostWatch() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// GHostMgr::Managable
	size_t hostOffset_{0};
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;

protected:
	QElapsedTimer et_;
	QMutex m_;
	int threadCount_{0};
	struct WatchThread : GThread {
		WatchThread(GHostWatch* hostWatch, GMac mac, GHostMgr::HostValue* hostValue);
		~WatchThread() override;
		void run() override;

		GHostWatch* hw_{nullptr};
		GMac mac_;
		GHostMgr::HostValue* hostValue_;
		GWaitEvent we_;
	};

protected:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		WatchThread* watchThread_{nullptr};
		Item(GHostWatch* hostWatch, GMac mac, GHostMgr::HostValue* hostValue) {
			watchThread_ = new WatchThread(hostWatch, mac, hostValue);
			watchThread_->start();
		}
		~Item() {
			if (watchThread_ != nullptr) {
				watchThread_->we_.wakeAll();
				watchThread_->quit();
				watchThread_->wait();
				delete watchThread_;
				watchThread_ = nullptr;
			}
		}
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

public:
	bool propLoad(QJsonObject jo, QMetaProperty mpro) override;
};
