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
    int channel = ieee80211_frequency_to_channel(freq / 1000000);
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
		sprintf(buf, "QProcess::execute(%s) return %d for channel %d", qPrintable(command), res, channel);
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
		sprintf(buf, "iw_set_ext(%s) return %d(%s) for channel %d", qPrintable(intfName_), res, strerror(errno), channel);
		error_ = buf;
		return false;
	}
	return true;
#endif
}

QList<int> GIw::channelList() {
	if (internalChannelList_.count() != 0)
		return internalChannelList_;

    QList<int> result;

    char buf[BufSize];
	QString command = "iw " + intfName_ + " info";
#ifdef Q_OS_ANDROID
	command = QString("su -c '%1'").arg(command);
#endif // Q_OS_ANDROID
	FILE* p = popen(qPrintable(command), "r");
    if (p == nullptr) {
		error_ = QString("fail to call %1").arg(command);
        return result;
    }

    int phyNo = -1;
    while (true) {
		if (std::fgets(buf, BufSize, p) == nullptr) break;
        int no;
        int res = sscanf(buf, "\twiphy %d", &no);
        if (res == 1) {
            phyNo = no;
            break;
        }
    }
    pclose(p);

    if (phyNo == -1) {
		error_ = "can not get phyNo";
        return result;
    }

	command = "iw phy" + QString::number(phyNo)+ " info";
#ifdef Q_OS_ANDROID
	command = QString("su -c '%1'").arg(command);
#endif // Q_OS_ANDROID
	p = popen(qPrintable(command), "r");
    if (p == nullptr) {
		error_ = QString("fail to call %1").arg(command);
        return result;
    }

    while (true) {
        if (std::fgets(buf, 256, p) == nullptr) break;
        int freq, channel;
		char additional[BufSize];
		int res = sscanf(buf, "\t\t* %d MHz [%d] (%s)", &freq, &channel, additional);
		if (res >= 2) {
			if (res >= 3 && strncmp(additional, "disabled", strlen("disabled")) == 0)
				continue;
			result.push_back(channel);
        }
    }
    pclose(p);

	internalChannelList_ = result;
	qDebug() << result;
	return result;
}

int GIw::ieee80211_frequency_to_channel(int freq) {
    /* see 802.11-2007 17.3.8.3.2 and Annex J */
    if (freq == 2484)
        return 14;
    /* see 802.11ax D6.1 27.3.23.2 and Annex E */
    else if (freq == 5935)
        return 2;
    else if (freq < 2484)
        return (freq - 2407) / 5;
    else if (freq >= 4910 && freq <= 4980)
        return (freq - 4000) / 5;
    else if (freq < 5950)
        return (freq - 5000) / 5;
    else if (freq <= 45000) /* DMG band lower limit */
        /* see 802.11ax D6.1 27.3.23.2 */
        return (freq - 5950) / 5;
    else if (freq >= 58320 && freq <= 70200)
        return (freq - 56160) / 2160;
    else
        return 0;
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
    EXPECT_FALSE(iw.setChannel(999));
}

TEST(GIw, channelListTest) {
    GIw iw("wlan0");
	QList<int> channelList = iw.channelList();
	std::cerr << "wlan0 channel list is ";
	for (int channel: channelList)
		std::cerr << channel << " ";
	std::cerr << std::endl;
}

#endif // GTEST
