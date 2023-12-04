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
struct G_EXPORT GIcmpHdr {
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

// PingRequest or PingReply
struct G_EXPORT GIcmpPingHdr : GIcmpHdr {
	uint16_t id_;
	uint16_t seq_;
	struct timeval ts_;
	uint32_t unused_;

	uint16_t id() { return ntohs(id_); }
	uint16_t seq() { return ntohs(id_); }
	struct timeval ts() {
		struct timeval res;
		res.tv_sec = ntohl(ts_.tv_sec);
		res.tv_usec = ntohl(ts_.tv_usec);
		return res;
	}
	void* data() { return pbyte(this) + sizeof(GIcmpPingHdr); }
};
typedef GIcmpPingHdr *PIcmpPingHdr;

struct G_EXPORT GIcmpTtlExceededHdr : GIcmpHdr {
	uint32_t unused_;
	GIpHdr* ipHdr() { return PIpHdr(pbyte(this) + sizeof(GIcmpTtlExceededHdr)); }
};
typedef GIcmpTtlExceededHdr *PIcmpTtlExceededHdr;

#pragma pack(pop)
