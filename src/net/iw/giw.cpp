#include "giw.h"
#include "base/gerr.h"
#include "iwlib.h"

#ifndef Q_OS_WIN

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
GIw::GIw() {
	skfd_ = iw_sockets_open();
	if (skfd_ < 0) {
		GLastErr lastErr;
		qDebug() << lastErr.msg();
	}
}

GIw::~GIw() {
	if (skfd_ != -1) {
		iw_sockets_close(skfd_);
		skfd_ = -1;
	}
}

int GIw::channel(QString intfName) {
	struct iwreq wrq;

	int res = iw_get_ext(skfd_, qPrintable(intfName), SIOCGIWFREQ, &wrq);
	if (res < 0) {
		GLastErr lastErr;
		qWarning() << QString("iw_get_ext for %1 return %2 %3").arg(intfName).arg(res).arg(lastErr.msg());
		return -1;
	}
	return wrq.u.freq.i;
}

bool GIw::setChannel(QString intfName, int channel) {
	struct iwreq wrq;

	double freq = channel;
	iw_float2freq(freq, &(wrq.u.freq));
	wrq.u.freq.flags = IW_FREQ_FIXED;
	int res = iw_set_ext(skfd_, qPrintable(intfName), SIOCSIWFREQ, &wrq);
	if (res < 0) {
		GLastErr lastErr;
		qWarning() << QString("iw_set_ext for %1 return %2 %3").arg(intfName).arg(res).arg(lastErr.msg());
		return false;
	}
	return true;
}

QList<int> GIw::channelList(QString intfName) {
	QList<int> result;

	struct iw_range	range;
	int res = iw_get_range_info(skfd_, qPrintable(intfName), &range);
	if (res < 0) {
		GLastErr lastErr;
		qWarning() << QString("iw_get_range_info for %1 return %2 %3").arg(intfName).arg(res).arg(lastErr.msg());
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
