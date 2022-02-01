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

#include "ghostdetect.h"

// ----------------------------------------------------------------------------
// GHostDelete
// ----------------------------------------------------------------------------
struct G_EXPORT GHostDelete : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(ulong checkInterval MEMBER checkInterval_)
	Q_PROPERTY(qint64 scanStartTimeout MEMBER scanStartTimeout_)
	Q_PROPERTY(ulong randomInterval MEMBER randomInterval_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(qint64 deleteTimeout MEMBER deleteTimeout_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)
	Q_PROPERTY(GObjPtr hostDetect READ getHostDetect WRITE setHostDetect)

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }
	GObjPtr getHostDetect() { return hostDetect_; }
	void setHostDetect(GObjPtr value) { hostDetect_ = dynamic_cast<GHostDetect*>(value.data()); }

public:
	bool enabled_{true};
	GDuration checkInterval_{1000}; // 1 sec
	qint64 scanStartTimeout_{60000}; // 60 secs
	GDuration randomInterval_{5000}; // 5 secs
	GDuration sendInterval_{1000}; // 1 sec
	qint64 deleteTimeout_{10000}; // 10 secs
	GPcapDevice* pcapDevice_{nullptr};
	GHostDetect* hostDetect_{nullptr};

public:
	Q_INVOKABLE GHostDelete(QObject* parent = nullptr);
	~GHostDelete() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	void checkRun();

	struct CheckThread : GThread {
		CheckThread(QObject *parent) : GThread(parent) {}
		~CheckThread() override {}
		GWaitEvent we_;
		void run() override {
			GHostDelete* hd = dynamic_cast<GHostDelete*>(parent());
			Q_ASSERT(hd != nullptr);
			hd->checkRun();
		}
	} checkThread_{this};

protected:
	struct ScanThread : GThread {
		ScanThread(GHostDelete* hostDelete, GHostDetect::Host* host);
		~ScanThread() override;
		void run() override;

		GHostDelete* hostDelete_{nullptr};
		GHostDetect::Host* host_{nullptr};
		GWaitEvent we_;
	};

	struct ScanThreadMap: QMap<GMac, ScanThread*> {
		QMutex m_;
	} stm_;

public:
	bool propLoad(QJsonObject jo, QMetaProperty mpro) override;
};
