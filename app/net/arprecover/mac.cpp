#include "mac.h"

Mac::Mac(const std::string& r) {
	std::string s;
	for(char ch: r) {
		if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
			s += ch;
	}
	int res = sscanf(s.c_str(), "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", &mac_[0], &mac_[1], &mac_[2], &mac_[3], &mac_[4], &mac_[5]);
	if (res != SIZE) {
		fprintf(stderr, "Mac::Mac sscanf return %d r=%s\n", res, r.c_str());
		return;
	}
}

Mac::operator std::string() const {
	char buf[20]; // enough size
	sprintf(buf, "%02x:%02X:%02X:%02X:%02X:%02X", mac_[0], mac_[1], mac_[2], mac_[3], mac_[4], mac_[5]);
	return std::string(buf);
}

Mac Mac::randomMac() {
	Mac res;
	for (int i = 0; i < SIZE; i++)
		res.mac_[i] = uint8_t(rand() % 256);
	res.mac_[0] &= 0x7F;
	return res;
}

Mac& Mac::nullMac() {
	static uint8_t _value[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static Mac res(_value);
	return res;
}

Mac& Mac::broadcastMac() {
	static uint8_t _value[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	static Mac res(_value);
	return res;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

static constexpr uint8_t _temp[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

TEST(Mac, ctorTest) {
	Mac mac1; // ()

	Mac mac2{mac1}; // (const Mac& r)

	Mac mac3(_temp); // (const uint8_t* r)

	Mac mac4(std::string("001122-334455")); // (const std::string& r)
	EXPECT_EQ(mac3, mac4);

	Mac mac5("001122-334455"); // (const std::string& r)
	EXPECT_EQ(mac3, mac5);
}

TEST(Mac, castingTest) {
	Mac mac("001122-334455");

	const uint8_t* uc = (uint8_t*)mac; // operator uint8_t*()
	uint8_t temp[Mac::SIZE];
	for (int i = 0; i < Mac::SIZE; i++)
		temp[i] = *uc++;
	EXPECT_TRUE(memcmp(&mac, temp, 6) == 0);

	std::string s2 = std::string(mac); // operator std::string()
	EXPECT_EQ(s2, "00:11:22:33:44:55");
}

TEST(Mac, funcTest) {
	Mac mac;

	mac.clear();
	EXPECT_TRUE(mac.isNull());

	mac = std::string("FF:FF:FF:FF:FF:FF");
	EXPECT_TRUE(mac.isBroadcast());

	mac = std::string("01:00:5E:00:11:22");
	EXPECT_TRUE(mac.isMulticast());
}

#include <map>
TEST(Mac, mapTest) {
	typedef std::map<Mac, int> MacMap;
	MacMap m;
	m.insert(std::make_pair(Mac("001122-334455"), 1));
	m.insert(std::make_pair(Mac("001122-334456"), 2));
	m.insert(std::make_pair(Mac("001122-334457"), 3));
	EXPECT_EQ(m.size(), 3);
	MacMap::iterator it = m.begin();
	EXPECT_EQ(it->second, 1); it++;
	EXPECT_EQ(it->second, 2); it++;
	EXPECT_EQ(it->second, 3);
}

#include <unordered_map>
TEST(Mac, unordered_mapTest) {
	typedef std::unordered_map<Mac, int> MacUnorderedMap;
	MacUnorderedMap m;
	m.insert(std::make_pair(Mac("001122-334455"), 1));
	m.insert(std::make_pair(Mac("001122-334456"), 2));
	m.insert(std::make_pair(Mac("001122-334457"), 3));
	//EXPECT_EQ(m.size(), 3);
}

#endif // GTEST
