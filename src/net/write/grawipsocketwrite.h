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

#include "net/write/gwrite.h"

// ----------------------------------------------------------------------------
// GRawIpSocketWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GRawIpSocketWrite : GWrite {
	Q_OBJECT
	Q_PROPERTY(int mtu MEMBER mtu_)

public:
	int mtu_{GPacket::MtuSize};

public:
	Q_INVOKABLE GRawIpSocketWrite(QObject* parent = nullptr) : GWrite(parent) {}
	~GRawIpSocketWrite() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	int sd_{0};

public:
	GPacket::Result write(GBuf buf) override;

public slots:
	GPacket::Result write(GPacket* packet) override;
};
