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
// GUdpHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GUdpHdr final {
	uint16_t sport_;
	uint16_t dport_;
	uint16_t len_;
	uint16_t sum_;

	uint16_t sport() { return ntohs(sport_); }
	uint16_t dport() { return ntohs(dport_); }
	uint16_t len() { return ntohs(len_); }
	uint16_t sum() { return ntohs(sum_); }

	static uint16_t calcSum(GIpHdr* ipHdr, GUdpHdr* udpHdr);
	static uint16_t inetCalcSum(GIpHdr* ipHdr, GUdpHdr* udpHdr);
	static GBuf parseData(GUdpHdr* udpHdr);
};
typedef GUdpHdr *PUdpHdr;
#pragma pack(pop)
