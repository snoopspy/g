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

#include "base/gstateobj.h"
#include "base/sys/gthread.h"
#include "base/sys/gwaitevent.h"

#ifndef Q_OS_WIN

#include "net/gnetinfo.h"
#include "net/iw/giw.h"

// ----------------------------------------------------------------------------
// GChannelHop
// ----------------------------------------------------------------------------
struct G_EXPORT GChannelHop : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(bool debugLog MEMBER debugLog_)
	Q_PROPERTY(QString intfName MEMBER intfName_)
	Q_PROPERTY(int hopInterval MEMBER hopInterval_)
	Q_PROPERTY(QStringList channelList MEMBER channelList_)

public:
	bool enabled_{true};
	bool debugLog_{false};
	QString intfName_{""};
	int hopInterval_{250};
	QStringList channelList_;

public:
	Q_INVOKABLE GChannelHop(QObject* parent = nullptr);
	~GChannelHop() override;

protected:
	GIw iw_;
	GStateWaitEvent swe_;

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

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};

#endif // Q_OS_WIN
