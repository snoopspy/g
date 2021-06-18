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
// GDhcpHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GDhcpHdr final {
	uint8_t type_; // Message type
	uint8_t hrd_; // Hardware type
	uint8_t hln_; // Hardware address length
	uint8_t hops_; // Hops
	uint32_t transaction_; // Transaction ID
	uint16_t elapsed_; // Seconds elapsed
	uint8_t bootp_; // Bootp flags
	GIp clientIp_; // Client IP address
	GIp yourIp_; // Your (client) IP address
	GIp serverIp_; // Next srever IP address
	GIp relayIp_; // Relay agent IP address
	GMac clientMac_; // Client MAC address
	uint8_t padding_[10]; // Client hardware address padding
	uint8_t serverHostName_[64]; // Server host name not given
	uint8_t bootFileName_[128]; // Boot file name not given
	uint32_t magic_; // Magic cookie : DHCP

	uint8_t type() { return type_; }
	uint8_t hrd() { return hrd_; }
	uint8_t hln() { return hln_; }
	uint8_t hops() { return hops_; }
	uint32_t transaction() { return ntohl(transaction_); }
	uint16_t elapsed() { return ntohs(elapsed_); }
	uint8_t bootp() { return bootp_; }
	GIp clientIp() { return ntohl(clientIp_); }
	GIp yourIp() { return ntohl(yourIp_); }
	GIp serverIp() { return ntohl(serverIp_); }
	GIp relayIp() { return ntohl(relayIp_); }
	GMac clientMac() { return clientMac_; }
	uint8_t* padding() { return padding_; }
	uint8_t* serverHostName() { return serverHostName_; }
	uint8_t* bootFileName() { return bootFileName_; }
	uint32_t magic() { return ntohl(magic_); }

	struct Option {
		uint8_t type_;
		uint8_t len_;

		Option* next() {
			Option* res = POption((char*)this + len_ + 2);
			if (res->type_ == End) return nullptr;
			return res;
		}

	};
	typedef Option *POption;

	Option* first() {
		return POption((char*)this + sizeof(GDhcpHdr));
	}

	struct OptionRequestIpAddress : Option {
		GIp ip_;
	};
	typedef OptionRequestIpAddress *POptionRequestIpAddress;

	// OptionType(Option::type_)
	enum: uint8_t {
		RequestedIpAddress = 50,
		End = 255
	};
};
typedef GDhcpHdr *PDhcpHdr;
#pragma pack(pop)
