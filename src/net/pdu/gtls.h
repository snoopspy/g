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
namespace GTls {
	struct Record {
		uint8_t contentType_;
		uint16_t version_;
		uint16_t length_;

		uint8_t contentType() { return contentType_; }
		uint16_t version() { return ntohs(version_); }
		uint16_t length() { return ntohs(length_); }

		// contentType_
		enum: uint16_t {
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

		// version_
		enum: uint16_t {
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
			Tls1_3_Fbd23 = 0xfb17, /** TLS 1.3 (Facebook draft 23) */
			Tls1_3_Fbd26 = 0xfb1a, /** TLS 1.3 (Facebook draft 26) */
			Unknown = 0 /** Unknown value */
		};
	};
	typedef Record *PRecord;

	struct Handshake {
		struct Length3 {
			uint8_t len_[3];
			operator quint32() const { // default
				return quint32(len_[0]) << 16 | quint32(len_[1]) << 8 | quint32(len_[2]);
			}
		};
		typedef Length3 *PLength3;

		uint8_t handshakeType_;
		Length3 length_;

		// handshakeType_
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

		static GTls::Handshake* check(GTls::Record* tr, uint32_t* size);
	};
	typedef Handshake *PHandshake;

	struct Extension {
		uint16_t type_;
		uint16_t length_;

		uint16_t type() { return ntohs(type_); }
		uint16_t length() { return htons(length_); }
		void* value() {
			return pbyte(this) + sizeof(type_) + sizeof(length_);
		}

		// type_
		enum: uint16_t {
			ServerName = 0,
			EcPointformats = 11,
			KeyShare = 51,
			SupportedVersions = 43
		};

		Extension* next() {
			gbyte* res = pbyte(this);
			res += sizeof(Extension) + length_;
			return reinterpret_cast<Extension*>(res);
		}
	};
	typedef Extension *PExtension;

	struct ServerNameIndication : GTls::Extension {
		uint16_t length_;
		uint8_t type_;
		uint16_t serverNameLength_;

		uint16_t length() { return htons(length_); }
		uint16_t type() { return type_; }
		uint16_t serverNameLength() { return htons(serverNameLength_); }
		QByteArray serverName() {
			char* p = pchar(this) + sizeof(ServerNameIndication);
			QByteArray res(p, serverNameLength());
			return res;
		}
	};
	typedef ServerNameIndication *PServerNameIndication;

	struct ClientHelloHs : Handshake {
		uint16_t version_;
		uint8_t random_[32];
		QByteArray sessionId_;
		QList<uint16_t> cipherSuites_;
		QByteArray compressionMethod_;
		QList<Extension*> extensions_;

		void parse(GTls::Handshake* hs);
	};

	struct ServerHelloHs : Handshake {
		uint16_t version_;
		uint8_t random_[32];
		QByteArray sessionId_;
		uint16_t cipherSuite_;
		QByteArray compressionMethod_;
		QList<Extension*> extensions_;

		void parse(GTls::Handshake* hs);
	};

	struct CertificateHs : Handshake {
		Handshake::Length3 certificatesLength_;
		QList<QByteArray> certificates_;

		void parse(GTls::Handshake* hs);
	};
};
#pragma pack(pop)
