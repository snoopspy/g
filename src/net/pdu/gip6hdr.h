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
// GIp6Hdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GIp6Hdr final {
	uint8_t v_tc1_;
	uint8_t tc2_fl_[3];
	uint16_t plen_;
	uint8_t nh_;
	uint8_t hl_;
	GIp6 sip_;
	GIp6 dip_;

	uint8_t v() { return (v_tc1_ & 0xF0) >> 4; }
	uint8_t tc() { return (v_tc1_ & 0x0F) << 4 | (tc2_fl_[0] & 0xF0) >> 4; }
	uint8_t fl() { return (tc2_fl_[0] & 0x0F) << 16 | tc2_fl_[1] << 8 | tc2_fl_[2]; }
	uint8_t plen() { return ntohs(plen_); }
	uint8_t nh() { return nh_; }
	uint8_t hl() { return hl_; }
	GIp6 sip() { return sip_; }
	GIp6 dip() { return dip_; }
};
typedef GIp6Hdr *PIp6Hdr;
#pragma pack(pop)
