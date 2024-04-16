#include "giw.h"

#include <QProcess>
#include <QDebug>

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
	intfName_ = intfName;
	channelList_.clear();

	char buf[BufSize];
	QString command = "iw " + intfName_ + " info";
#ifdef Q_OS_ANDROID
	command = QString("su -c '%1'").arg(command);
#endif // Q_OS_ANDROID
	FILE* p = popen(qPrintable(command), "r");
	if (p == nullptr) {
		error_ = QString("fail to call %1").arg(command);
		return false;
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
		return false;
	}

	command = "iw phy" + QString::number(phyNo)+ " info";
#ifdef Q_OS_ANDROID
	command = QString("su -c '%1'").arg(command);
#endif // Q_OS_ANDROID
	p = popen(qPrintable(command), "r");
	if (p == nullptr) {
		error_ = QString("fail to call %1").arg(command);
		return false;
	}

	while (true) {
		if (std::fgets(buf, BufSize, p) == nullptr) break;
		int freq, channel;
		char additional[BufSize];
		int res = sscanf(buf, "\t\t* %d MHz [%d] (%s)", &freq, &channel, additional);
		if (res >= 2) {
			if (res >= 3 && strncmp(additional, "disabled", strlen("disabled")) == 0)
				continue;
			channelList_.push_back(channel);
		}
	}
	pclose(p);

	return true;
}

bool GIw::close() {
	return true;
}

int GIw::channel() {
	QString command = QString("iw %1 info").arg(intfName_);
	FILE* p = popen(qPrintable(command), "r");
	if (p == nullptr) {
		error_ = QString("fail to call %1").arg(command);
		return InvalidChannel;
	}

	int result = InvalidChannel;
	char buf[BufSize];
	while (true) {
		if (std::fgets(buf, BufSize, p) == nullptr) break;
		int channel, freq;
		int res = sscanf(buf, "\tchannel %d (%d MHz)", &channel, &freq);
		if (res == 2) {
			if (freqToChannel(freq) == channel) {
				result = channel;
				break;
			}
			qWarning() << QString("Invalid channel(%1) and freq(%2) %3").arg(channel).arg(freq).arg(buf);
		}
	}
	pclose(p);
	return result;
}

bool GIw::setChannel(int channel) {
#ifndef Q_OS_ANDROID
	QString command = QString("iw %1 set channel %2").arg(intfName_).arg(channel);
#else
	QString command = QString("nexutil -k%1").arg(channel);
#endif

	QString program = "su";
	QStringList arguments {
		"-c",
		command
	};

	int res = QProcess::execute(program, arguments);
	if (res != 0) {
		char buf[BufSize];
		sprintf(buf, "QProcess::execute(%s) return %d for channel %d", qPrintable(command), res, channel);
		error_ = buf;
		return false;
	}
	return true;
}

// ieee80211_frequency_to_channel
int GIw::freqToChannel(int freq) {
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
	EXPECT_TRUE(iw.setChannel(2));
	EXPECT_TRUE(iw.setChannel(3));
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
