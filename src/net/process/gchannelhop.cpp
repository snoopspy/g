#include "gchannelhop.h"

#ifndef Q_OS_WIN

// ----------------------------------------------------------------------------
// GChannelHop
// ----------------------------------------------------------------------------
GChannelHop::GChannelHop(QObject* parent) : GStateObj(parent) {
#ifndef Q_OS_ANDROID
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GIntf* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
#else
	intfName_ = "wlan0";
#endif // Q_OS_ANDROID
}

GChannelHop::~GChannelHop() {
	close();
}

bool GChannelHop::doOpen() {
	if (!enabled_) return true;

	if (!iw_.open(qPrintable(intfName_))) {
		SET_ERR(GErr::Fail, iw_.error_);
		return false;
	}
	if (firstOpen_) {
		firstOpen_ = false;
		qDebug() << iw_.channelList();
	}
	thread_.start();
	return true;
}

bool GChannelHop::doClose() {
	if (!enabled_) return true;

	swe_.wakeAll();
	thread_.quit();
	bool res = thread_.wait();
	iw_.close();
	return res;
}

void GChannelHop::run() {
	qDebug() << "beg"; // gilgil temp 2022.01.24
	QList<int> channelList;
	for (QString& c: channelList_) {
		int channel = c.toInt();
		if (channel != 0)
			channelList.push_back(channel);
	}
	if (channelList.count() == 0) {
		QList<int> cl = iw_.channelList();
		if (cl.count() == 0) {
			SET_ERR(GErr::Fail, QString("channel count is zero for %1 %2").arg(intfName_, iw_.error_));
		}
		if (cl.count() % 5 == 0)
			cl.push_back(1);

		QList<int>::iterator it = cl.begin();
		while (true) {
			channelList.push_back(*it);
			if (channelList.count() == cl.count()) break;
			for (int i = 0; i < 5; i++) {
				if (++it == cl.end()) it = cl.begin();
			}
		}
	}

	while (active()) {
		for (int channel: channelList) {
			if (!active()) break;
			if (!iw_.setChannel(channel)) {
				qDebug() << iw_.error_;
			}
			if (debugLog_)
				qDebug() << "channel " << channel;
			emit channelChanged(channel);
			if (swe_.wait(hopInterval_)) break;
		}
	}
	qDebug() << "end"; // gilgil temp 2022.01.24
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GChannelHop::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		return new GPropItemInterface(param);
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB

#endif // Q_OS_WIN
