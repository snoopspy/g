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

#include <QDebug>
#include <QList>
#ifdef Q_OS_ANDROID
#include <QProcess>
#endif

#include "iwlib.h"

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
struct GIw {
public:
	GIw();
	GIw(QString intfName);
	virtual ~GIw();

protected:
	int skfd_{-1};
	struct iw_range	range_;
	QString intfName_;

public:
	static const int BufSize = 256;
	QString error_;

public:
	bool open(QString intfName);
	bool close();

	int channel();
	bool setChannel(int channel);
	QList<int> channelList();
};

#endif // Q_OS_WIN
