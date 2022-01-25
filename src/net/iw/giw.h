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

#include <list>
#include <string>
#include "iwlib.h"
#ifdef Q_OS_ANDROID
#include "net/demon/gdemonclient.h"
#endif

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
struct GIw {
public:
	GIw();
	GIw(std::string intfName);
	virtual ~GIw();

protected:
	int skfd_{-1};
	struct iw_range	range_;
	std::string intfName_;

public:
	static const int BufSize = 256;
	std::string error_;

public:
	bool open(std::string intfName);
	bool close();

	int channel();
	bool setChannel(int channel);
	std::list<int> channelList();

#ifdef Q_OS_ANDROID
protected:
  GDemonClient* demonClient_{nullptr};
#endif
};

#endif // Q_OS_WIN
