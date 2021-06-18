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

#include <QThread>
#include "base/gbase.h"

// ----------------------------------------------------------------------------
// GThread
// ----------------------------------------------------------------------------
struct G_EXPORT GThread : QThread {
	GThread(QObject *parent = nullptr) : QThread(parent) {}
	void start(Priority priority = InheritPriority);
	bool wait(GDuration timeout = G::Timeout);
};

// ----------------------------------------------------------------------------
// GThreadMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GThreadMgr {
	friend struct GThread;

	static void suspendStart();
	static void resumeStart();

private: // singleton
	GThreadMgr();
	virtual ~GThreadMgr();

	bool suspended_{false};

	struct ThreadInfo {
		QThread* thread_;
		QThread::Priority priority_;
	};
	QList<ThreadInfo> threadInfos_;
	static GThreadMgr& instance() {
		static GThreadMgr threadMgr;
		return threadMgr;
	}
};
