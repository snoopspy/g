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
// GMac
// ----------------------------------------------------------------------------
struct G_EXPORT GMac final {
	static constexpr int SIZE = 6;

	// constructor
	GMac() {}
	GMac(const GMac& r) { memcpy(this->mac_, r.mac_, SIZE); }
	GMac(const gbyte* r) { memcpy(this->mac_, r, SIZE); }
	GMac(const QString& r);

	// assign operator
	GMac& operator = (const GMac& r) { memcpy(this->mac_, r.mac_, SIZE); return *this; }

	// casting operator
	explicit operator gbyte*() const { return const_cast<gbyte*>(mac_); }
	explicit operator QString() const;

	// comparison operator
	bool operator == (const GMac& r) const { return memcmp(mac_, r.mac_, SIZE) == 0; }
	bool operator != (const GMac& r) const { return memcmp(mac_, r.mac_, SIZE) != 0; }
	bool operator < (const GMac& r) const { return memcmp(mac_, r.mac_, SIZE) < 0; }
	bool operator > (const GMac& r) const { return memcmp(mac_, r.mac_, SIZE) > 0; }
	bool operator <= (const GMac& r) const { return memcmp(mac_, r.mac_, SIZE) <= 0; }
	bool operator >= (const GMac& r) const { return memcmp(mac_, r.mac_, SIZE) >= 0; }
	bool operator == (const pbyte r) const { return memcmp(mac_, r, SIZE) == 0; }

public:
	void clear() {
		for (int i = 0; i < SIZE; i++) mac_[i] = 0;
	}

	bool isNull() const {
		for (int i = 0; i < SIZE; i++) if (mac_[i] != 0) return false;
		return true;
	}

	bool isBroadcast() const { // FF:FF:FF:FF:FF:FF
		for (int i = 0; i < SIZE; i++) if (mac_[i] != 0xFF) return false;
		return true;
	}

	bool isMulticast() const { // 01:00:5E:0*
		return mac_[0] == 0x01 && mac_[1] == 0x00 && mac_[2] == 0x5E && (mac_[3] & 0x80) == 0x00;
	}

	static GMac randomMac();
	static GMac& nullMac();
	static GMac& broadcastMac();

protected:
	gbyte mac_[SIZE];
};

namespace std {
	template<>
	struct hash<GMac> {
		size_t operator() (const GMac& r) const {
#ifdef Q_OS_ANDROID
			gbyte* p = pbyte(&r);
			size_t res = 0;
			for(size_t i = 0; i < GMac::SIZE; ++i) res = res * 31 + size_t(*p++);
			return res;
#else // Q_OS_ANDROID
			return std::_Hash_impl::hash(&r, GMac::SIZE);
#endif // Q_OS_ANDROID
		}
	};
}
