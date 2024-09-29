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
// GArpHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GArpHdr final {
	uint16_t hrd_;
	uint16_t pro_;
	uint8_t hln_;
	uint8_t pln_;
	uint16_t op_;
	GMac smac_;
	GIp sip_;
	GMac tmac_;
	GIp tip_;

	uint16_t hrd() { return ntohs(hrd_); }
	uint16_t pro() { return ntohs(pro_); }
	uint8_t hln() { return hln_; }
	uint8_t pln() { return pln_; }
	uint16_t op() { return ntohs(op_);}
	GMac smac() { return smac_; }
	GIp sip() { return ntohl(sip_); }
	GMac tmac() { return tmac_; }
	GIp tip() { return ntohl(tip_); }

	// hrd_
	enum: uint16_t {
		Netrom = 0, // from KA9Q: NET/ROM pseudo
		Ether = 1, // Ethernet 10Mbps
		Eether = 2, // Experimental Ethernet
		Ax25 = 3, // AX.25 Level 2
		Pponet = 4, // PROnet token ring
		Chaos = 5, // Chaosnet
		Ieee802 = 6, // IEEE 802.2 Ethernet/TR/TB
		Arcnet = 7, // ARCnet
		Aaaletalk = 8, // APPLEtalk
		Lanstar = 9, // Lanstar
		Dlci = 15, // Frame Relay DLCI
		Atm = 19, // ATM
		Metricom = 23, // Metricom STRIP (new IANA id)
		Ipsec = 31 // IPsec tunnel
	};

	// op_
	enum: uint16_t {
		Request = 1, // req to resolve address
		Reply = 2, // resp to previous request
		RevRequest = 3, // req protocol address given hardware
		RevReply = 4, // resp giving protocol address
		InvRequest = 8, // req to identify peer
		InvReply = 9 // resp identifying peer
	};
};
typedef GArpHdr *PArpHdr;
#pragma pack(pop)
