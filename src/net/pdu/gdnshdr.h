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

	// ------------------------------------------------------------------------
	// Type
	// ------------------------------------------------------------------------
	enum: uint16_t { // From RFC 1035
		TypeA = 1, // 1 Address
		TypeNS, // 2 Name Server
		TypeMD, // 3 Mail Destination
		TypeMF, // 4 Mail Forwarder
		TypeCNAME, // 5 Canonical Name
		TypeSOA, // 6 Start of Authority
		TypeMB, // 7 Mailbox
		TypeMG, // 8 Mail Group
		TypeMR, // 9 Mail Rename
		TypeNULL, // 10 NULL RR
		TypeWKS, // 11 Well-known-service
		TypePTR, // 12 Domain name pointer
		TypeHINFO, // 13 Host information
		TypeMINFO, // 14 Mailbox information
		TypeMX, // 15 Mail Exchanger
		TypeTXT, // 16 Arbitrary text string
		TypeRP, // 17 Responsible person
		TypeAFSDB, // 18 AFS cell database
		TypeX25, // 19 X_25 calling address
		TypeISDN, // 20 ISDN calling address
		TypeRT, // 21 Router
		TypeNSAP, // 22 NSAP address
		TypeNSAP_PTR, // 23 Reverse NSAP lookup (deprecated)
		TypeSIG, // 24 Security signature
		TypeKEY, // 25 Security key
		TypePX, // 26 X.400 mail mapping
		TypeGPOS, // 27 Geographical position (withdrawn)
		TypeAAAA, // 28 IPv6 Address
		TypeLOC, // 29 Location Information
		TypeNXT, // 30 Next domain (security)
		TypeEID, // 31 Endpoint identifier
		TypeNIMLOC, // 32 Nimrod Locator
		TypeSRV, // 33 Service record
		TypeATMA, // 34 ATM Address
		TypeNAPTR, // 35 Naming Authority PoinTeR
		TypeKX, // 36 Key Exchange
		TypeCERT, // 37 Certification record
		TypeA6, // 38 IPv6 Address (deprecated)
		TypeDNAME, // 39 Non-terminal DNAME (for IPv6)
		TypeSINK, // 40 Kitchen sink (experimental)
		TypeOPT, // 41 EDNS0 option (meta-RR)
		TypeAPL, // 42 Address Prefix List
		TypeDS, // 43 Delegation Signer
		TypeSSHFP, // 44 SSH Key Fingerprint
		TypeIPSECKEY, // 45 IPSECKEY
		TypeRRSIG, // 46 RRSIG
		TypeNSEC, // 47 Denial of Existence
		TypeDNSKEY, // 48 DNSKEY
		TypeDHCID, // 49 DHCP Client Identifier
		TypeNSEC3, // 50 Hashed Authenticated Denial of Existence
		TypeNSEC3PARAM, // 51 Hashed Authenticated Denial of Existence
		TypeHIP = 55, // 55 Host Identity Protocol
		TypeSPF = 99, // 99 Sender Policy Framework for E-Mail
		TypeUINFO, // 100 IANA-Reserved
		TypeUID, // 101 IANA-Reserved
		TypeGID, // 102 IANA-Reserved
		TypeUNSPEC, // 103 IANA-Reserved
		TypeTKEY = 249,	// 249 Transaction key
		TypeTSIG, // 250 Transaction signature
		TypeIXFR, // 251 Incremental zone transfer
		TypeAXFR, // 252 Transfer zone of authority
		TypeMAILB, // 253 Transfer mailbox records
		TypeMAILA, // 254 Transfer mail agent records
		TypeANY // Not a DNS type, but a DNS query type, meaning "all types"
	};
};
typedef GDnsHdr *PDnsHdr;
#pragma pack(pop)
