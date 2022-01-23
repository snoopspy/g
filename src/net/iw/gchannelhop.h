// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#ifndef Q_OS_WIN

#include "base/gstateobj.h"
#include "base/sys/gthread.h"
#include "base/sys/gwaitevent.h"

#include "giw.h"

// ----------------------------------------------------------------------------
// GChannelHop
// ----------------------------------------------------------------------------
struct G_EXPORT GChannelHop : GStateObj {
	Q_OBJECT
	Q_PROPERTY(QString intfName MEMBER intfName_)
	Q_PROPERTY(int hopInterval MEMBER hopInterval_)
	Q_PROPERTY(QStringList channelList MEMBER channelList_)

public:
	QString intfName_{""};
	int hopInterval_{1000};
	QStringList channelList_;

public:
	GChannelHop(QObject* parent = nullptr);
	~GChannelHop() override;

protected:
	GIw iw_;
	GWaitEvent we_;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	struct Thread : GThread {
		Thread(GChannelHop* channelHop) : GThread(channelHop) {}
	protected:
		void run() override {
			GChannelHop* channelHop = dynamic_cast<GChannelHop*>(parent());
			Q_ASSERT(channelHop != nullptr);
			channelHop->run();
		}
	} thread_{this};

	virtual void run();

signals:
	void channelChanged(int channel);
};

#endif // Q_OS_WIN
