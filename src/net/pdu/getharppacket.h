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

#include "gethhdr.h"
#include "garphdr.h"

// ----------------------------------------------------------------------------
// GEthArpPacket
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct GEthArpPacket final {
	GEthHdr ethHdr_;
	GArpHdr arpHdr_;

	void init(GMac dmac, GMac smac, uint16_t operation, GMac senderMac, GIp senderIp, GMac targetMac, GIp targetIp);
};
#pragma pack(pop)
