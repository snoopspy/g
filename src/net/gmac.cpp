#include "gmac.h"

// ----------------------------------------------------------------------------
// GMac
// ----------------------------------------------------------------------------
GMac::GMac(const QString& r) {
	std::string s;
	for (QChar ch : r) {
		if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
			s += ch.toLatin1();
	}
	int res = sscanf(s.c_str(), "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", &mac_[0], &mac_[1], &mac_[2], &mac_[3], &mac_[4], &mac_[5]);
	if (res != SIZE) {
		qWarning() << QString("sscanf(%1) return %2").arg(r).arg(res);
		memset(mac_, 0, SIZE);
	}
}

GMac::operator QString() const {
	char s[20]; // enough size
	sprintf(s, "%02X:%02X:%02X:%02X:%02X:%02X", mac_[0], mac_[1], mac_[2], mac_[3], mac_[4], mac_[5]);
	return QString(s);
}

GMac GMac::randomMac() {
	GMac res;
	for (int i = 0; i < SIZE; i++)
		res.mac_[i] = gbyte(rand() % 256);
	res.mac_[0] &= 0x7F;
	return res;
}

GMac& GMac::nullMac() {
	static gbyte _value[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static GMac res(_value);
	return res;
}

GMac& GMac::broadcastMac() {
	static gbyte _value[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	static GMac res(_value);
	return res;
}

uint qHash(const GMac& mac) {
#ifdef Q_OS_ANDROID
	gbyte* p = pbyte(&mac);
	size_t res = 0;
	for(size_t i = 0; i < GMac::SIZE; ++i) res = res * 31 + size_t(*p++);
	return res;
#else // Q_OS_ANDROID
	return std::_Hash_impl::hash(&mac, GMac::SIZE);
#endif // Q_OS_ANDROID
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

static constexpr gbyte _temp[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

TEST(GMac, ctorTest) {
	GMac mac1; // ()

	GMac mac2{mac1}; // (const GMac& r)

	GMac mac3(_temp); // (const gbyte* r)

	GMac mac4(QString("001122-334455")); // (const QString& r)
	EXPECT_EQ(mac3, mac4);

	GMac mac5("001122-334455"); // (const QString& r)
	EXPECT_EQ(mac3, mac5);
}

TEST(GMac, castingTest) {
	GMac mac("001122-334455");

	const gbyte* uc = (gbyte*)mac; // operator gbyte*()
	gbyte temp[GMac::SIZE];
	for (int i = 0; i < GMac::SIZE; i++)
		temp[i] = *uc++;
	EXPECT_TRUE(memcmp(&mac, temp, 6) == 0);

	QString s2 = QString(mac); // operator QString()
	EXPECT_EQ(s2, "00:11:22:33:44:55");
}

TEST(GMac, funcTest) {
	GMac mac;

	mac.clear();
	EXPECT_TRUE(mac.isNull());

	mac = QString("FF:FF:FF:FF:FF:FF");
	EXPECT_TRUE(mac.isBroadcast());

	mac = QString("01:00:5E:00:11:22");
	EXPECT_TRUE(mac.isMulticast());
}

#include <map>
TEST(GMac, mapTest) {
	typedef std::map<GMac, int> MacMap;
	MacMap m;
	m.insert(std::make_pair(GMac("001122-334455"), 1));
	m.insert(std::make_pair(GMac("001122-334456"), 2));
	m.insert(std::make_pair(GMac("001122-334457"), 3));
	EXPECT_EQ(m.size(), 3);
	MacMap::iterator it = m.begin();
	EXPECT_EQ(it->second, 1); it++;
	EXPECT_EQ(it->second, 2); it++;
	EXPECT_EQ(it->second, 3);
}

#include <unordered_map>
TEST(GMac, unordered_mapTest) {
	typedef std::unordered_map<GMac, int> MacUnorderedMap;
	MacUnorderedMap m;
	m.insert(std::make_pair(GMac("001122-334455"), 1));
	m.insert(std::make_pair(GMac("001122-334456"), 2));
	m.insert(std::make_pair(GMac("001122-334457"), 3));
	//EXPECT_EQ(m.size(), 3);
}

#endif // GTEST
