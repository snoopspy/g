#include "gchannelhop.h"

#ifndef Q_OS_WIN

// ----------------------------------------------------------------------------
// GChannelHop
// ----------------------------------------------------------------------------
GChannelHop::GChannelHop(QObject* parent) : GStateObj(parent) {
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GIntf* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
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
	thread_.start();
	return true;
}

bool GChannelHop::doClose() {
	if (!enabled_) return true;

	we_.wakeAll();
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
	qDebug() << channelList; // gilgil temp 2022.01.24
	while (active()) {
		for (int channel: channelList) {
			if (!active()) break;
			if (!iw_.setChannel(channel)) {
				qDebug() << iw_.error_;
				break;
			}
			emit channelChanged(channel);
			if (we_.wait(hopInterval_)) break;
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
