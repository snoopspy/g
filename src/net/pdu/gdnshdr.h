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
// GDnsHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GDnsHdr final {
	uint16_t id_;
	uint16_t flags_;
	uint16_t num_q_;
	uint16_t num_answ_rr;
	uint16_t num_auth_rr;
	uint16_t num_addi_rr;

	uint16_t id() { return ntohs(id_); }
	uint16_t flags() { return ntohs(flags_); }
	uint16_t q() { return ntohs(num_q_); }
	uint16_t answ() { return ntohs(num_answ_rr); }
	uint16_t auth() { return ntohs(num_auth_rr); }
	uint16_t addr() { return ntohs(num_addi_rr); }
};
typedef GDnsHdr *PDnsHdr;
#pragma pack(pop)
