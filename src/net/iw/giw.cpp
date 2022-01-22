#include "giw.h"
#include <errno.h>
#include <string.h>

#ifndef Q_OS_WIN

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
GIw::GIw() {
	skfd_ = iw_sockets_open();
	if (skfd_ < 0) {
		fprintf(stderr, "iw_sockets_open return %d(%s)\n", skfd_, strerror(errno));
	}
}

GIw::~GIw() {
	if (skfd_ != -1) {
		iw_sockets_close(skfd_);
		skfd_ = -1;
	}
}

int GIw::channel(std::string intfName) {
	error_ = "";

	struct iw_range	range;
	int res = iw_get_range_info(skfd_, intfName.data(), &range);
	if(res < 0) {
		char buf[256];
		sprintf(buf, "iw_get_range_info(%s) return %d(%s)\n", intfName.data(), res, strerror(errno));
		error_ = buf;
		return -1;
	}

	struct iwreq wrq;
	res = iw_get_ext(skfd_, intfName.data(), SIOCGIWFREQ, &wrq);
	if (res < 0) {
		char buf[256];
		sprintf(buf, "iw_get_ext(%s) return %d(%s)\n", intfName.data(), res, strerror(errno));
		return -1;
	}

	double freq = iw_freq2float(&(wrq.u.freq));
	int channel = iw_freq_to_channel(freq, &range);
	return channel;
}

bool GIw::setChannel(std::string intfName, int channel) {
	error_ = "";

	double freq = channel;
	struct iwreq wrq;
	iw_float2freq(freq, &(wrq.u.freq));
	wrq.u.freq.flags = IW_FREQ_FIXED;
	int res = iw_set_ext(skfd_, intfName.data(), SIOCSIWFREQ, &wrq);
	if (res < 0) {
		char buf[256];
		sprintf(buf, "iw_set_ext(%s) return %d(%s)\n", intfName.data(), res, strerror(errno));
		error_ = buf;
		return false;
	}
	return true;
}

std::list<int> GIw::channelList(std::string intfName) {
	std::list<int> result;

	struct iw_range	range;
	int res = iw_get_range_info(skfd_, intfName.data(), &range);
	if (res < 0) {
		char buf[256];
		sprintf(buf, "iw_get_range_info(%s) return %d(%s)\n", intfName.data(), res, strerror(errno));
		error_ = buf;
		return result;
	}

	for(int i = 0; i < range.num_frequency; i++)
		result.push_back(range.freq[i].i);

	return result;
}

GIw& GIw::instance() {
	static GIw iw;
	return iw;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(GIw, channelTest) {
	GIw& iw = GIw::instance();
	qDebug() << "wlan0 channel is " << iw.channel("wlan0");
}

TEST(GIw, setChannelTest) {
	GIw& iw = GIw::instance();
	EXPECT_TRUE(iw.setChannel("wlan0", 1));
	//EXPECT_FALSE(iw.setChannel("wlan0", 999));
}

TEST(GIw, channelListTest) {
	GIw& iw = GIw::instance();
	qDebug() << "wlan0 channel list is " << iw.channelList("wlan0");
}

#endif // GTEST

#endif // Q_OS_WIN
