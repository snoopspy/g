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
// GBlock
// ----------------------------------------------------------------------------
struct G_EXPORT GBlock : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)

public:
	bool enabled_{true};

public:
	Q_INVOKABLE GBlock(QObject* parent = nullptr) : GStateObj(parent) {}
	~GBlock() override {}

protected:
	bool doOpen() override { return true; }
	bool doClose() override { return true; }

public slots:
	void block(GPacket* packet);
	void unblock(GPacket* packet);

signals:
	void blocked(GPacket* packet);
	void unblocked(GPacket* packet);
};
