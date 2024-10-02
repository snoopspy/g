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

#include "net/packet/ganypacket.h"

// ----------------------------------------------------------------------------
// GWritable
// ----------------------------------------------------------------------------
struct GWritable {
	virtual GPacket::Result writeBuf(GBuf buf) = 0;
	virtual GPacket::Result write(GPacket* packet) = 0;
	GPacket::Result writeMtuSplit(GPacket* packet, size_t mtu, GPacket::Dlt dlt, GDuration usleepTime = 100);

	GAnyPacket anyPacket_;
	QByteArray sendByteArray_;
};
