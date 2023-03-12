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
// GTimeStampSyncDelay
// ----------------------------------------------------------------------------
struct G_EXPORT GTimeStampSyncDelay : GStateObj {
	Q_OBJECT
	Q_PROPERTY(double speed MEMBER speed_) // ratio

public:
	double speed_{1.};

protected:
	GStateWaitEvent swe_;
	QElapsedTimer et_;
	qint64 lastClock_;
	qint64 lastTs_;

public:
	Q_INVOKABLE GTimeStampSyncDelay(QObject* parent = nullptr) : GStateObj(parent) {}
	~GTimeStampSyncDelay() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public slots:
	bool delay(GPacket* packet);

signals:
	void delayed(GPacket* packet);
};
