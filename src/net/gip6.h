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

uint qHash(const GIp6& ip6);

namespace std {
	template<>
	struct hash<GIp6> {
		size_t operator() (const GIp6& ip6) const {
#ifdef Q_OS_ANDROID
			gbyte* p = pbyte(&ip6);
			size_t res = 0;
			for(size_t i = 0; i < GIp6::SIZE; ++i) res = res * 31 + size_t(*p++);
			return res;
#else // Q_OS_ANDROID
			return std::_Hash_impl::hash(&ip6, GIp6::SIZE);
#endif // Q_OS_ANDROID
		}
	};
}
typedef GIp6 *PIp6;
