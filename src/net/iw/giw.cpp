#include "giw.h"
#include <errno.h>
#include <string.h>

#ifndef Q_OS_WIN

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
GIw::GIw() {
}

GIw::GIw(std::string intfName) {
	open(intfName);
}

GIw::~GIw() {
	close();
#ifdef Q_OS_ANDROID
	if (demonClient_ != nullptr) {
		delete demonClient_;
		demonClient_ = nullptr;
	}
#endif
}

bool GIw::open(std::string intfName) {
	error_ = "";
	if (skfd_ != -1) {
		char buf[BufSize];
		sprintf(buf, "skfd_ is already opened\n");
		error_ = buf;
		return false;
	}

	skfd_ = iw_sockets_open();
	if (skfd_ < 0) {
		char buf[BufSize];
		sprintf(buf, "iw_sockets_open return %d(%s)\n", skfd_, strerror(errno));
		error_ = buf;
		return false;
	}

	int res = iw_get_range_info(skfd_, intfName.data(), &range_);
	if(res < 0) {
		char buf[256];
		sprintf(buf, "iw_get_range_info(%s) return %d(%s)\n", intfName.data(), res, strerror(errno));
		error_ = buf;
		return false;
	}

	intfName_ = intfName;
	return true;
}

bool GIw::close() {
	if (skfd_ != -1) {
		iw_sockets_close(skfd_);
		skfd_ = -1;
	}
	return true;
}

int GIw::channel() {
	struct iwreq wrq;
	int res = iw_get_ext(skfd_, intfName_.data(), SIOCGIWFREQ, &wrq);
	if (res < 0) {
		char buf[256];
		sprintf(buf, "iw_get_ext(%s) return %d(%s)\n", intfName_.data(), res, strerror(errno));
		error_ = buf;
		return -1;
	}

	double freq = iw_freq2float(&(wrq.u.freq));
	int channel = iw_freq_to_channel(freq, &range_);
	return channel;
}

#ifdef Q_OS_ANDROID
#include "net/demon/gtrace.h"
bool GIw::setChannel(int channel) {
	if (demonClient_ == nullptr) {
		demonClient_ = new GDemonClient("127.0.0.1", GDemon::DefaultPort);
		GDemon::ChOpenRes res = demonClient_->chOpen(intfName_);
		if (!res.result_) {
			GTRACE("%s", demonClient_->error_.data());
		  return false;
		}
	}
	demonClient_->chSetChannel(channel);
}
#else
bool GIw::setChannel(int channel) {
	double freq = channel;
	struct iwreq wrq;
	iw_float2freq(freq, &(wrq.u.freq));
	wrq.u.freq.flags = IW_FREQ_FIXED;
	int res = iw_set_ext(skfd_, intfName_.data(), SIOCSIWFREQ, &wrq);
	if (res < 0) {
		char buf[BufSize];
		sprintf(buf, "iw_set_ext(%s) return %d(%s)\n", intfName_.data(), res, strerror(errno));
		error_ = buf;
		return false;
	}
	return true;
}
#endif

std::list<int> GIw::channelList() {
	std::list<int> result;
	for(int i = 0; i < range_.num_frequency; i++)
		result.push_back(range_.freq[i].i);
	return result;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(GIw, channelTest) {
	GIw iw("wlan0");
	std::cerr << "wlan0 channel is " << iw.channel() << std::endl;
	int channel = iw.channel();
	EXPECT_NE(channel, -1);
}

TEST(GIw, setChannelTest) {
	GIw iw("wlan0");
	EXPECT_TRUE(iw.setChannel(1));
	// EXPECT_FALSE(iw.setChannel(999));
}

TEST(GIw, channelListTest) {
	GIw iw("wlan0");
	std::list<int> channelList = iw.channelList();
	std::cerr << "wlan0 channel list is ";
	for (int channel: channelList)
		std::cerr << channel << " ";
	std::cerr << std::endl;
}

#endif // GTEST

#endif // Q_OS_WIN
