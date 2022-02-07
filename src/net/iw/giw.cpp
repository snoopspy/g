#include "giw.h"
#include <errno.h>
#include <string.h>

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
GIw::GIw() {
}

GIw::GIw(QString intfName) {
	open(intfName);
}

GIw::~GIw() {
	close();
}

bool GIw::open(QString intfName) {
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

	int res = iw_get_range_info(skfd_, qPrintable(intfName), &range_);
	if(res < 0) {
		char buf[256];
		sprintf(buf, "iw_get_range_info(%s) return %d(%s)\n", qPrintable(intfName), res, strerror(errno));
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
	int res = iw_get_ext(skfd_, qPrintable(intfName_), SIOCGIWFREQ, &wrq);
	if (res < 0) {
		char buf[256];
		sprintf(buf, "iw_get_ext(%s) return %d(%s)\n", qPrintable(intfName_), res, strerror(errno));
		error_ = buf;
		return -1;
	}

	double freq = iw_freq2float(&(wrq.u.freq));
	int channel = iw_freq_to_channel(freq, &range_);
	return channel;
}

bool GIw::setChannel(int channel) {
#ifdef Q_OS_ANDROID
	QString command = QString("nexutil -k%1").arg(channel);

	QString program = "su";
	QStringList arguments {
		"-c",
		command
	};
	int res = QProcess::execute(program, arguments);
	// qDebug() << program << arguments << res; // gilgil temp 2022.01.26
	if (res != 0) {
		char buf[BufSize];
		sprintf(buf, "QProcess::execute(%s) return %d\n", qPrintable(command), res);
		error_ = buf;
		return false;
	}
	return true;
#else
	double freq = channel;
	struct iwreq wrq;
	iw_float2freq(freq, &(wrq.u.freq));
	wrq.u.freq.flags = IW_FREQ_FIXED;
	int res = iw_set_ext(skfd_, qPrintable(intfName_), SIOCSIWFREQ, &wrq);
	if (res < 0) {
		char buf[BufSize];
		sprintf(buf, "iw_set_ext(%s) return %d(%s)\n", qPrintable(intfName_), res, strerror(errno));
		error_ = buf;
		return false;
	}
	return true;
#endif
}

QList<int> GIw::channelList() {
	QList<int> result;
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
	GIw iw("mon0");
	std::cerr << "mon0 channel is " << iw.channel() << std::endl;
	int channel = iw.channel();
	EXPECT_NE(channel, -1);
}

TEST(GIw, setChannelTest) {
	GIw iw("mon0");
	EXPECT_TRUE(iw.setChannel(1));
	// EXPECT_FALSE(iw.setChannel(999));
}

TEST(GIw, channelListTest) {
	GIw iw("mon0");
	QList<int> channelList = iw.channelList();
	std::cerr << "wlan0 channel list is ";
	for (int channel: channelList)
		std::cerr << channel << " ";
	std::cerr << std::endl;
}

#endif // GTEST
