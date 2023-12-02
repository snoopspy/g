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

#include "net/packet/gethpacket.h"
#include "net/packet/gippacket.h"
#include "net/packet/gdot11packet.h"
#include "net/packet/gnullpacket.h"

// ----------------------------------------------------------------------------
// GAnyPacket
// ----------------------------------------------------------------------------
struct GAnyPacket {
	GPacket* get(GPacket::Dlt dlt);

protected:
	GEthPacket ethPacket_;
	GIpPacket ipPacket_;
	GDot11Packet dot11Packet_;
	GNullPacket nullPacket_;
};
