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

#include "net/packet/gpacket.h"
#include "net/packet/gethpacket.h"
#include "net/packet/gippacket.h"

// ----------------------------------------------------------------------------
// GWritable
// ----------------------------------------------------------------------------
struct GWritable {
	virtual GPacket::Result write(GBuf buf) = 0;
	virtual GPacket::Result write(GPacket* packet) = 0;
	GPacket::Result writeMtuSplit(GPacket* packet, size_t mtu, GPacket::Dlt dlt, GDuration usleepTime = 100);

	QByteArray sendByteArray_;
	GEthPacket sendEthPacket_;
	GIpPacket sendIpPacket_;
};
