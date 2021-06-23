#pragma once

#include <cstdint>
#include <cstring>
#include <string>

// ----------------------------------------------------------------------------
// Mac
// ----------------------------------------------------------------------------
struct Mac final {
	static constexpr int SIZE = 6;

	// constructor
	Mac() {}
	Mac(const Mac& r) { memcpy(this->mac_, r.mac_, SIZE); }
	Mac(const uint8_t* r) { memcpy(this->mac_, r, SIZE); }
	Mac(const std::string& r);

	// assign operator
	Mac& operator = (const Mac& r) { memcpy(this->mac_, r.mac_, SIZE); return *this; }

	// casting operator
	explicit operator uint8_t*() const { return const_cast<uint8_t*>(mac_); }
	explicit operator std::string() const;

	// comparison operator
	bool operator == (const Mac& r) const { return memcmp(mac_, r.mac_, SIZE) == 0; }
	bool operator != (const Mac& r) const { return memcmp(mac_, r.mac_, SIZE) != 0; }
	bool operator < (const Mac& r) const { return memcmp(mac_, r.mac_, SIZE) < 0; }
	bool operator > (const Mac& r) const { return memcmp(mac_, r.mac_, SIZE) > 0; }
	bool operator <= (const Mac& r) const { return memcmp(mac_, r.mac_, SIZE) <= 0; }
	bool operator >= (const Mac& r) const { return memcmp(mac_, r.mac_, SIZE) >= 0; }
	bool operator == (const uint8_t* r) const { return memcmp(mac_, r, SIZE) == 0; }

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

	static Mac randomMac();
	static Mac& nullMac();
	static Mac& broadcastMac();

protected:
	uint8_t mac_[SIZE];
};

namespace std {
	template<>
	struct hash<Mac> {
		typedef unsigned char byte;
		typedef unsigned char *pbyte;
		size_t operator() (const Mac& r) const {
#ifdef __ANDROID__
			byte* p = pbyte(&r);
			size_t res = 0;
			for(size_t i = 0; i < Mac::SIZE; ++i) res = res * 31 + size_t(*p++);
			return res;
#else // __ANDROID__
			return std::_Hash_impl::hash(&r, Mac::SIZE);
#endif // __ANDROID__
		}
	};
}
