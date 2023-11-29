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

#include "giphdr.h"

// ----------------------------------------------------------------------------
// GIcmpHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GIcmpHdr final {
	uint8_t type_;
	uint8_t code_;
	uint16_t sum_;

	uint8_t type() { return type_; }
	uint8_t code() { return code_; }
	uint16_t sum() { return ntohs(sum_); }

	// Type(type_)
	enum: uint8_t {
		PingReply = 0,
		DestinationUnReaclable = 3,
		PingRequest = 8,
		TtlExceeded = 11
	};

	static uint16_t calcChecksum(GIpHdr* ipHdr, GIcmpHdr* icmpHdr);
};
typedef GIcmpHdr *PIcmpHdr;
#pragma pack(pop)
