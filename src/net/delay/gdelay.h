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
#include "net/packet/gpacket.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GDelay
// ----------------------------------------------------------------------------
struct G_EXPORT GDelay : GStateObj {
	Q_OBJECT
	Q_PROPERTY(ulong timeout MEMBER timeout_) // msec

public:
	GDuration timeout_{1000};

protected:
	GStateWaitEvent swe_;

public:
	Q_INVOKABLE GDelay(QObject* parent = nullptr) : GStateObj(parent) {}
	~GDelay() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public slots:
	bool delay(GPacket* packet);

signals:
	void delayed(GPacket* packet);
};
