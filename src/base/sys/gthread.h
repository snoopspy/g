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
	GThread(QObject* parent = nullptr) : QThread(parent) {}
	~GThread() override {}

	Priority priority_{InheritPriority};
	bool startRequired_{false};

	void start();
	bool wait(GDuration timeout = G::Timeout);
};

// ----------------------------------------------------------------------------
// GThreadMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GThreadMgr {
	friend struct GThread;
	friend struct GStateObj;

	static void suspendStart();
	static void resumeStart(bool resume = true);

private: // singleton
	GThreadMgr() {}
	virtual ~GThreadMgr() {}

	bool suspended_{false};

	QList<GThread*> threads_;

public:
	static GThreadMgr& instance() {
		static GThreadMgr threadMgr;
		return threadMgr;
	}
};
