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

#include <pcap.h>
#include <QObject>
#include "net/pdu/gpdu.h"
#include "net/pdu/gethhdr.h"
#include "net/pdu/garphdr.h"
#include "net/pdu/giphdr.h"
#include "net/pdu/gip6hdr.h"
#include "net/pdu/gtcphdr.h"
#include "net/pdu/gudphdr.h"
#include "net/pdu/gicmphdr.h"

// ----------------------------------------------------------------------------
// GPacket
// ----------------------------------------------------------------------------
struct G_EXPORT GPacket : QObject {
	Q_OBJECT
	Q_ENUMS(Dlt)

public:
	// --------------------------------------------------------------------------
	// Result
	// --------------------------------------------------------------------------
	typedef enum {
		Eof = -2, // read
		Fail = -1, // read write
		TimeOut = 0, // read
		Ok = 1, // read write
	} Result;

	// --------------------------------------------------------------------------
	// Dlt(DataLinkType)
	// --------------------------------------------------------------------------
	typedef enum {
		Eth, // DLT_EN10MB (1)
		Ip, // DLT_RAW (228)
		Dot11, // DLT_IEEE802_11_RADIO (127)
		Null, // DLT_NULL (0)
	} Dlt;
	static QString dltToString(Dlt dlt);
	static int dltToInt(Dlt dlt);
	static Dlt intToDlt(int dataLink);

public:
	GPacket(QObject* parent = nullptr) : QObject(parent) { clear(); } // parent may be GCapture
	GPacket(const GPacket& r);
	~GPacket() override {}
	GPacket& operator = (const GPacket& r);

protected:
	Dlt dlt_{Null};

public:
	//
	// info
	//
	Dlt dlt() { return dlt_; };

	//
	// sniffing
	//
	struct timeval ts_;
	GBuf buf_;

	//
	// control
	//
	struct {
		bool block_{false};
		bool changed_{false};
	} ctrl;

	//
	// header
	//
	GEthHdr* ethHdr_{nullptr};
	GArpHdr* arpHdr_{nullptr};

	GIpHdr* ipHdr_{nullptr};
	GIp6Hdr* ip6Hdr_{nullptr};

	GTcpHdr* tcpHdr_{nullptr};
	GUdpHdr* udpHdr_{nullptr};
	GIcmpHdr* icmpHdr_{nullptr};

	GBuf tcpData_;
	GBuf udpData_;

	//
	// constant
	//
	static constexpr int MaxBufSize = 65536;

#ifdef _DEBUG
	//
	// debug
	//
	bool parsed_;
#endif // _DEBUG

public:
	void clear() {
		ts_.tv_sec = 0;
		ts_.tv_usec = 0;
		buf_.clear();
		ctrl.block_ = false;
		ctrl.changed_ = false;
		ethHdr_ = nullptr;
		arpHdr_ = nullptr;
		ipHdr_ = nullptr;
		ip6Hdr_ = nullptr;
		tcpHdr_ = nullptr;
		udpHdr_ = nullptr;
		icmpHdr_ = nullptr;
		tcpData_.clear();
		udpData_.clear();
#ifdef _DEBUG
		parsed_ = false;
#endif // _DEBUG
	}

	explicit operator QString() const;
	virtual void parse();

	void copyFrom(GPacket* source, GBuf newBuf);
};
typedef GPacket *PPacket;
