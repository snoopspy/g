#include "base/gerr.h"
#include "gip.h"
#include <cstdlib> // for atoi

// ----------------------------------------------------------------------------
// GIp
// ----------------------------------------------------------------------------
GIp::GIp(const QString& r) {
	std::string s = r.toStdString();
	pbyte p = pbyte(&ip_);
	int res = sscanf(s.c_str(), "%hhu.%hhu.%hhu.%hhu", &p[0], &p[1], &p[2], &p[3]);
	if (res != SIZE) {
		qWarning() << QString("sscanf(%1) return %2").arg(r).arg(res);
		ip_ = 0;
		return;
	}
	ip_ = ntohl(ip_);
}

GIp::operator QString() const {
	char s[INET_ADDRSTRLEN];
	sprintf(s, "%d.%d.%d.%d",
		(ip_ & 0xFF000000) >> 24,
		(ip_ & 0x00FF0000) >> 16,
		(ip_ & 0x0000FF00) >> 8,
		(ip_ & 0x000000FF) >> 0);
	return QString(s);
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(GIp, ctorTest) {
	GIp ip1; // ()

	GIp ip2{ip1}; // (const GIp& r)

	GIp ip3{0x7F000001}; // (const uint32_t r)
	EXPECT_EQ(ip3, 0x7F000001);

	GIp ip4{QString("127.0.0.1")}; // (const QString& r)
	EXPECT_EQ(ip4, 0x7F000001);

	GIp ip5{"127.0.0.1"}; // (const QString& r)
	EXPECT_EQ(ip5, 0x7F000001);
}

TEST(GIp, castingTest) {
	GIp ip{"127.0.0.1"};

	uint32_t ui; ui = ip; // operator uint32_t()
	EXPECT_EQ(ui, 0x7F000001);

	QString s2; s2 = QString(ip); // operator QString()
	EXPECT_EQ(s2, "127.0.0.1");
}

TEST(GIp, funcTest) {
	GIp ip;

	ip.clear();
	EXPECT_EQ(ip, 0);

	ip = QString("127.0.0.1");
	EXPECT_TRUE(ip.isLocalHost());

	ip = QString("255.255.255.255");
	EXPECT_TRUE(ip.isBroadcast());

	ip = QString("224.0.0.0");
	EXPECT_TRUE(ip.isMulticast());
}

#endif // GTEST
