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
	Q_PROPERTY(ulong rescanInterval MEMBER rescanInterval_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(int scanCount MEMBER scanCount_)
	Q_PROPERTY(QString scanNetmask MEMBER scanNetmask_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }

public:
	bool enabled_{true};
	GDuration rescanInterval_{600000}; // 10 minutes
	GDuration sendInterval_{10}; // 10 msecs
	int scanCount_{3};
	QString scanNetmask_{"FFFFFF00"};
	GPcapDevice* pcapDevice_{nullptr};

public:
	Q_INVOKABLE GHostScan(QObject* parent = nullptr);
	~GHostScan() override;

protected:
	GStateWaitEvent swe_;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	bool propLoad(QJsonObject jo, QMetaProperty mpro) override;

protected:
	struct ScanThread: GThread {
		ScanThread(QObject *parent) : GThread(parent) {}
		~ScanThread() {}
		void run() override {
			GHostScan* has = dynamic_cast<GHostScan*>(parent());
			has->run();
		}
	} scanThread_{this};

	void run();

};
