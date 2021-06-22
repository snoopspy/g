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

#include "gpdu.h"

// ----------------------------------------------------------------------------
// GIpHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GIpHdr final { // libnet_ipv4_hdr // gilgil temp 2019.05.13
	uint8_t v_hl_;
	uint8_t tos_;
	uint16_t len_;
	uint16_t id_;
	uint16_t off_;
	uint8_t ttl_;
	uint8_t p_;
	uint16_t sum_;
	GIp sip_;
	GIp dip_;

	uint8_t v() { return (v_hl_ & 0xF0) >> 4; }
	uint8_t hl() { return v_hl_ & 0x0F; }
	uint8_t tos() { return tos_; }
	uint16_t len() { return ntohs(len_); }
	uint16_t id() { return ntohs(id_); }
	uint16_t off() { return ntohs(off_); }
	uint8_t ttl() { return ttl_; }
	uint8_t p() { return p_; }
	uint16_t sum() { return ntohs(sum_); }
	GIp sip() { return ntohl(sip_); }
	GIp dip() { return ntohl(dip_); }

	// Protocol(p_)
	enum: uint8_t {
		Icmp = 1, // Internet Control Message Protocol
		Igmp = 2, // Internet Group Management Protocol
		Tcp = 6, // Transmission Control Protocol
		Udp = 17, // User Datagram Protocol
		Sctp = 132, // Stream Control Transport Protocol
	};

	static uint16_t calcChecksum(GIpHdr* ipHdr);
	static uint16_t recalcChecksum(uint16_t oldChecksum, uint16_t oldValue, uint16_t newValue);
	static uint16_t recalcChecksum(uint16_t oldChecksum, uint32_t oldValue, uint32_t newValue);
};
typedef GIpHdr *PIpHdr;
#pragma pack(pop)
