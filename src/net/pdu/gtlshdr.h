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
// GTlsHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct G_EXPORT GTlsHdr final {
	uint16_t contentType_;
	uint16_t version_;
	uint16_t length_;

	// ContentType(contentType_)
	enum ContentType : uint16_t {
		Unassigned = 0, // 0x00 (0 - 19)
		ChangeCipherSpec = 20, // 0x14
		Alert = 21, // 0x15
		Handshake = 22, // 0x16
		ApplicationData = 23, // 0x17
		Heartbeat = 24, // 0x18
		Tls12Cid = 25, // 0x19
		Ack = 26, // 0x1A
		ReturnRoutabilityCheck = 27, // 0x1B
		Unassigned2 = 28, // 0x1C (28 - 31)
		Reserved = 32, // 0x20 (32 - 63)
	};

	// Version(version_)
	enum Version : uint16_t {
		Ssl2 = 0x0200, /** SSL 2.0 */
		Ssl3 = 0x0300, /** SSL 3.0 */
		Tls1_0 = 0x0301, /** TLS 1.0 */
		Tls1_1 = 0x0302, /** TLS 1.1 */
		Tls1_2 = 0x0303, /** TLS 1.2 */
		Tls1_3 = 0x0304, /** TLS 1.3 */
		Tls1_3_D14 = 0x7f0e, /** TLS 1.3 (draft 14) */
		Tls1_3_D15 = 0x7f0f, /** TLS 1.3 (draft 15) */
		Tls1_3_D16 = 0x7f10, /** TLS 1.3 (draft 16) */
		Tls1_3_D17 = 0x7f11, /** TLS 1.3 (draft 17) */
		Tls1_3_D18 = 0x7f12, /** TLS 1.3 (draft 18) */
		Tls1_3_D19 = 0x7f13, /** TLS 1.3 (draft 19) */
		Tls1_3_D20 = 0x7f14, /** TLS 1.3 (draft 20) */
		Tls1_3_D21 = 0x7f15, /** TLS 1.3 (draft 21) */
		Tls1_3_D22 = 0x7f16, /** TLS 1.3 (draft 22) */
		Tls1_3_D23 = 0x7f17, /** TLS 1.3 (draft 23) */
		Tls1_3_D24 = 0x7f18, /** TLS 1.3 (draft 24) */
		Tls1_3_D25 = 0x7f19, /** TLS 1.3 (draft 25) */
		Tls1_3_D26 = 0x7f1a, /** TLS 1.3 (draft 26) */
		Tls1_3_D27 = 0x7f1b, /** TLS 1.3 (draft 27) */
		Tls1_3_D28 = 0x7f1c, /** TLS 1.3 (draft 28) */
		Tls1_3_FBD23 = 0xfb17, /** TLS 1.3 (Facebook draft 23) */
		Tls1_3_FBD26 = 0xfb1a, /** TLS 1.3 (Facebook draft 26) */
		Unknown = 0 /** Unknown value */
	};

	// HandleShakeType(handshakeType_)
	enum: uint8_t {
		HelloRequest = 0, // 0x00
		ClientHello = 1, // 0x01
		ServerHello = 2, // 0x02
		NetSessionTicket = 4, // 0x04
		Certificate = 11, // 0x0B
		ServerKeyExchange = 12, // 0x0C
		CertificateRequest = 13, // 0x0D
		ServerHelloDone = 14, // 0x0E
		CertificateVerify = 15, // 0x0F
		ClientKeyExchange = 16, // 0x10
		Finished = 20, // 0x14
		CertificateStatus = 22 // 0x16
	};

	struct HandshakeHdr {
		uint8_t handshakeType_;
		uint8_t length_[3];
	};

	struct ClientHelloHdr : HandshakeHdr {
		uint16_t version_;
		uint8_t random[32];
		uint8_t sessionIdLength_;
	protected:
		// sessionId_
		uint16_t cipherSuitesLength_;
		uint16_t* cipherSuites_;

	};

};
typedef GTlsHdr *PTlsHdr;
#pragma pack(pop)
