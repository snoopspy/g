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
// GTcpHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GTcpHdr final {
	uint16_t sport_;
	uint16_t dport_;
	uint32_t seq_;
	uint32_t ack_;
	uint8_t off_rsvd_;
	uint8_t flags_;
	uint16_t win_;
	uint16_t sum_;
	uint16_t urp_;

	uint16_t sport() { return ntohs(sport_); }
	uint16_t dport() { return ntohs(dport_); }
	uint32_t seq() { return ntohl(seq_); }
	uint32_t ack() { return ntohl(ack_); }
	uint8_t off() { return (off_rsvd_ & 0xF0) >> 4; }
	uint8_t rsvd() { return off_rsvd_ & 0x0F; }
	uint8_t flags() { return flags_; }
	uint16_t win() { return ntohs(win_); }
	uint16_t sum() { return ntohs(sum_); }
	uint16_t urp() { return ntohs(urp_); }

	// flag_
	enum: uint8_t {
		Urg = 0x20,
		Ack = 0x10,
		Psh = 0x08,
		Rst = 0x04,
		Syn = 0x02,
		Fin = 0x01
	};

	static uint16_t calcChecksum(GIpHdr* ipHdr, GTcpHdr* tcpHdr);
	static GBuf parseData(GIpHdr* ipHdr, GTcpHdr* tcpHdr);
};
typedef GTcpHdr *PTcpHdr;
#pragma pack(pop)
