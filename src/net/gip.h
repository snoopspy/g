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

	bool isPrivate() const {
		if ((ip_ & 0xFFFF0000) == 0xC0A80000) return true; // 192.168.0.0/16 (192.168.0.0 ~ 192.168.255.255)
		if ((ip_ & 0xFFF00000) == 0xAC100000) return true; // 172.16.0.0/12 (172.16.0.0 ~ 172.31.255.255)
		if ((ip_ & 0xFF000000) == 0x0A000000) return true; // 10.0.0.0/8 (10.0.0.0 ~ 10.255.255.255)
		return false;
	}

	bool isLocalHost() const { // 127.0.0.0/8 (127.0.0.0 ~ 127.255.255.255.255)
		return (ip_ & 0xFF000000) == 0x7F000000;
	}

	bool isBroadcast() const { // 255.255.255.0/8 (255.0.0.0 ~ 255.255.255.255)
		return (ip_ & 0xFF000000) == 0xFF000000;
	}

	bool isMulticast() const { // 224.0.0.0/4 (224.0.0.0 ~ 239.255.255.255)
		return (ip_ & 0xF0000000) == 0xE0000000;
	}

	bool isUnassigned() const { // 169.254.0.0/16 (169.254.0.0 ~ 169.254.255.255)
		return (ip_ & 0xFFFF0000) == 0xA9FE0000;
	}

protected:
	quint32 ip_;
};
typedef GIp *PIp;
