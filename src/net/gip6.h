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
// GIp6
// ----------------------------------------------------------------------------
struct G_EXPORT GIp6 final {
	static constexpr int SIZE = 16;

	// constructor
	GIp6() {}
	GIp6(const GIp6& r) { memcpy(ip6_, r.ip6_, SIZE); }
	GIp6(const gbyte* r) { memcpy(ip6_, r, SIZE); }
	GIp6(const QString& r);

	// assign operator
	GIp6& operator = (const GIp6& r) { memcpy(this->ip6_, r.ip6_, SIZE); return *this; }

	// casting operator
	operator gbyte*() const { return const_cast<gbyte*>(ip6_); } // default casting operator
	explicit operator QString() const;

	// comparison operator
	bool operator == (const GIp6& r) const { return memcmp(ip6_, r.ip6_, SIZE) == 0; }
	bool operator != (const GIp6& r) const { return memcmp(ip6_, r.ip6_, SIZE) != 0; }
	bool operator < (const GIp6& r) const { return memcmp(ip6_, r.ip6_, SIZE) < 0; }
	bool operator > (const GIp6& r) const { return memcmp(ip6_, r.ip6_, SIZE) > 0; }
	bool operator <= (const GIp6& r) const { return memcmp(ip6_, r.ip6_, SIZE) <= 0; }
	bool operator >= (const GIp6& r) const { return memcmp(ip6_, r.ip6_, SIZE) >= 0; }
	bool operator == (const u_char* r) const { return memcmp(ip6_, r, SIZE) == 0; }

	void clear() {
		memset(ip6_, 0, SIZE);
	}

	bool isLocalHost() {
		return true; // gilgil temp 2019.05.11
	}

	bool isBroadcast() {
		return true; // gilgil temp 2019.05.11
	}

	bool isMulticast() {
		return true; // gilgil temp 2019.05.11
	}

protected:
	gbyte ip6_[SIZE];
};

namespace std {
	template<>
	struct hash<GIp6> {
		size_t operator() (const GIp6& r) const {
			return std::_Hash_impl::hash(&r, GIp6::SIZE);
		}
	};
}
