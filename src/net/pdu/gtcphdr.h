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

	static uint16_t calcSum(GIpHdr* ipHdr, GTcpHdr* tcpHdr);
	static uint16_t inetCalcSum(GIpHdr* ipHdr, GTcpHdr* tcpHdr);
	static GBuf parseData(GIpHdr* ipHdr, GTcpHdr* tcpHdr);

	//
	// Option
	//
	struct Option {
		uint8_t kind_;
		uint8_t len_;

		void* value() {
			return pbyte(this) + sizeof(Option);
		}

		Option* next() {
			gbyte* res = pbyte(this);
			if (*res == Nop)
				return POption(++res);
			res += this->len_;
			return POption(res);
		}
	};
	typedef Option* POption;

	Option* firstOption() {
		return POption(pbyte(this) + sizeof(GTcpHdr));
	}

	// option kind
	enum: uint8_t {
		Eof = 0,
		Nop = 1,
		Mss = 2,
		WindowScale = 3,
		SackPermitted = 4,
		Sack = 5,
		Echo = 6,
		EchoReply = 7,
		TimeStamps = 8
	};
};
typedef GTcpHdr *PTcpHdr;
#pragma pack(pop)
