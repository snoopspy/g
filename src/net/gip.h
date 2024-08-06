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

#include "gnet.h"

// ----------------------------------------------------------------------------
// GIp
// ----------------------------------------------------------------------------
struct G_EXPORT GIp final {
	static constexpr int Size = 4;

	// constructor
	GIp() {}
	GIp(const GIp& r) : ip_(r.ip_) {}
	GIp(const quint32 r) : ip_(r) {}
	GIp(const QString& r);

	// assign operator
	GIp& operator = (const GIp& r) { ip_ = r.ip_; return *this; }

	// casting operator
	operator quint32() const { return ip_; } // default
	explicit operator QString() const;

	void clear() {
		ip_ = 0;
	}

	bool isNull() const {
		return ip_ == 0;
	}

	bool isLocalHost() const { // 127.*.*.*
		uint8_t prefix = (ip_ & 0xFF000000) >> 24;
		return prefix == 0x7F;
	}

	bool isBroadcast() const { // 255.255.255.255
		return ip_ == 0xFFFFFFFF;
	}

	bool isMulticast() const { // 224.0.0.0 ~ 239.255.255.255
		uint8_t prefix = (ip_ & 0xFF000000) >> 24;
		return prefix >= 0xE0 && prefix < 0xF0;
	}

	bool isPrivate() const {
		if ((ip_ & 0xFFFF0000) == 0xC0A80000) return true; // 192.168.0.0/16
		if ((ip_ & 0xFFFF0000) == 0xAC200000) return true; // 172.16.0.0/16
		if ((ip_ & 0xFF000000) == 0x0A000000) return true; // 10.0.0.0/8
		return false;
	}

protected:
	quint32 ip_;
};
typedef GIp *PIp;
