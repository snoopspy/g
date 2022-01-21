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

#include "net/gnet.h"

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
struct G_EXPORT GIw {
private: // for singleton
	GIw();
	virtual ~GIw();

protected:
	int skfd_{-1};

public:
	int channel(QString intfName);
	bool setChannel(QString intfName, int channel);
	QList<int> channelList(QString intfName);

public:
	static GIw& instance();
};

#endif
