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

// ----------------------------------------------------------------------------
// GPacketDebug
// ----------------------------------------------------------------------------
struct G_EXPORT GPacketDebug : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)

public:
	bool enabled_{true};

public:
	Q_INVOKABLE GPacketDebug(QObject* parent = nullptr) : GStateObj(parent) {}
	~GPacketDebug() override { close(); }

protected:
	int frameNo_;

protected:
	bool doOpen() override;
	bool doClose() override;

public slots:
	void debug(GPacket* packet);

signals:
	void debugged(GPacket* packet);
};
