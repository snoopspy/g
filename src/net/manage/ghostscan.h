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
#include "base/sys/gwaitevent.h"
#include "base/sys/gthread.h"
#include "net/capture/gpcapdevice.h"

// ----------------------------------------------------------------------------
// GHostScan
// ----------------------------------------------------------------------------
struct G_EXPORT GHostScan : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(ulong rescanSleepTime MEMBER rescanSleepTime_)
	Q_PROPERTY(ulong sendSleepTime MEMBER sendSleepTime_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }

public:
	bool enabled_{true};
	GDuration rescanSleepTime_{600000}; // 10 minutes
	GDuration sendSleepTime_{50}; // 50 msecs
	GPcapDevice* pcapDevice_{nullptr};

public:
	Q_INVOKABLE GHostScan(QObject* parent = nullptr);
	~GHostScan() override;

protected:
	GWaitEvent we_;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	void run();

	struct ScanThread: GThread {
		ScanThread(QObject *parent) : GThread(parent) {}
		~ScanThread() {}
		void run() override {
			GHostScan* has = dynamic_cast<GHostScan*>(parent());
			has->run();
		}
	} scanThread_{this};

public:
	bool propLoad(QJsonObject jo, QMetaProperty mpro) override;

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
