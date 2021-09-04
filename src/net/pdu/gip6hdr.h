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
	uint8_t v_pr_;
	uint8_t flags_[3];
	uint16_t en_;
	uint8_t nh_;
	uint8_t hl_;
	GIp6 sip_;
	GIp6 dip_;

	uint8_t v() { return (v_pr_ & 0xF0) >> 4; }
	uint8_t pr() { return v_pr_ & 0x0F; }
	uint8_t* flags() { return flags_; }
	uint8_t nh() { return nh_; }
	uint8_t hl() { return hl_; }
	GIp6 sip() { return sip_; }
	GIp6 dip() { return dip_; }
};
typedef GIp6Hdr *PIp6Hdr;
#pragma pack(pop)
