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

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
struct GIw {
private: // for singleton
	GIw();
	virtual ~GIw();

protected:
	int skfd_{-1};

public:
	std::string error_;

public:
	int channel(std::string intfName);
	bool setChannel(std::string intfName, int channel);
	std::list<int> channelList(std::string intfName);

public:
	static GIw& instance();
};

#endif
