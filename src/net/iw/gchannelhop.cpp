#include "gchannelhop.h"

#ifndef Q_OS_WIN

// ----------------------------------------------------------------------------
// GChannelHop
// ----------------------------------------------------------------------------
GChannelHop::GChannelHop(QObject* parent) : GStateObj(parent) {
}

GChannelHop::~GChannelHop() {
	close();
}

bool GChannelHop::doOpen() {
	if (!iw_.open(qPrintable(intfName_))) {
		SET_ERR(GErr::Fail, iw_.error_.data());
		return false;
	}
	thread_.start();
	return true;
}

bool GChannelHop::doClose() {
	iw_.close();
	we_.wakeAll();
	thread_.quit();
	bool res = thread_.wait();
	return res;
}

void GChannelHop::run() {
	qDebug() << "beg"; // gilgil temp 2022.01.24
	QList<int> channelList;
	for (QString c: channelList_) {
		int channel = c.toInt();
		if (channel != 0)
			channelList.push_back(channel);
	}
	if (channelList.count() == 0) {
		std::list<int> cl = iw_.channelList();
		for (int c: cl)
			channelList.push_back(c);
	}
	qDebug() << channelList; // gilgil temp 2022.01.24
	while (active()) {
		for (int channel: channelList) {
			if (!iw_.setChannel(channel))
				qDebug() << iw_.error_.data();
			qDebug() << "current channel =" << channel; // gilgil temp 2022.01.24
			emit channelChanged(channel);
			if (we_.wait(hopInterval_)) break;
		}
	}
	qDebug() << "end"; // gilgil temp 2022.01.24
}

#endif // Q_OS_WIN
