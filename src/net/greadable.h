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

// ----------------------------------------------------------------------------
// GReadable
// ----------------------------------------------------------------------------
struct GReadable {
	virtual GPacket::Result read(GPacket* packet) = 0;
};
