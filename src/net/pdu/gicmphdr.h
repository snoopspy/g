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

	// type_
	enum: uint8_t {
		PingReply = 0,
		DestinationUnReaclable = 3,
		PingRequest = 8,
		TtlExceeded = 11
	};

	static uint16_t calcChecksum(GIpHdr* ipHdr, GIcmpHdr* icmpHdr);
	static uint16_t inetCalcChecksum(GIpHdr* ipHdr, GIcmpHdr* icmpHdr);
};
typedef GIcmpHdr *PIcmpHdr;

// for PingRequest and PingReply
struct G_EXPORT GIcmpPingHdr : GIcmpHdr {
	uint16_t id_;
	uint16_t seq_;
	uint32_t tv_sec_;
	uint32_t tv_usec_;

	uint16_t id() { return ntohs(id_); }
	uint16_t seq() { return ntohs(seq_); }
	uint32_t tv_sec() { return ntohl(tv_sec_); }
	uint32_t tv_usec() { return ntohl(tv_usec_); }
	struct timeval ts() {
		struct timeval tv;
		tv.tv_sec = tv_sec();
		tv.tv_usec = tv_usec();
		return tv;
	}

	gbyte* data() { return pbyte(this) + sizeof(GIcmpPingHdr); }
};
typedef GIcmpPingHdr *PIcmpPingHdr;

// for DestinationUnReaclable and TtlExceeded
struct G_EXPORT GIcmpIpHdr : GIcmpHdr {
	uint32_t unused_;

	uint32_t unused() { return ntohl(unused_); }
	GIpHdr* ipHdr() { return PIpHdr(pbyte(this) + sizeof(GIcmpIpHdr)); }
};
typedef GIcmpIpHdr *PIcmpIpHdr;

#pragma pack(pop)
