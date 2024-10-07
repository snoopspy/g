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

#include <QRandomGenerator>

#include "base/gstateobj.h"
#include "base/sys/gthread.h"
#include "net/packet/gpacket.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GThreadDelay
// ----------------------------------------------------------------------------
struct G_EXPORT GThreadDelay : GStateObj {
	Q_OBJECT
	Q_PROPERTY(ulong timeout MEMBER timeout_) // msec
	Q_PROPERTY(ulong jitter MEMBER jitter_) // msec
	Q_PROPERTY(bool flushOnClose MEMBER flushOnClose_) // msec

public:
	GDuration timeout_{1000};
	GDuration jitter_{0};
	bool flushOnClose_{true};

public:
	Q_INVOKABLE GThreadDelay(QObject* parent = nullptr) : GStateObj(parent) {}
	~GThreadDelay() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	QRandomGenerator rg_;

protected:
	struct BufferThread : GThread, QRecursiveMutex {
		BufferThread(GThreadDelay* threadDelay) : GThread(threadDelay), threadDelay_(threadDelay) {}
		void run() override;
		GThreadDelay* threadDelay_;
		GStateWaitEvent swe_;
		struct BufferMap : QMap<qint64 /*timestamp*/, GPacket* /*packet*/> {} map_;
	} bufferThread_{this};

public slots:
	void delay(GPacket* packet);

signals:
	void delayed(GPacket* packet);
};
