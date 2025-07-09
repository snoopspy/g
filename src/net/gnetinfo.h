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

#include "net/gintf.h"
#include "net/grtm.h"

// ----------------------------------------------------------------------------
// GNetInfo
// ----------------------------------------------------------------------------
struct G_EXPORT GNetInfo {
protected:
	GNetInfo() { init(); }
	virtual ~GNetInfo() {}

	GIntfList intfList_;
	GRtm rtm_;

public:
	GIntfList& intfList() { return intfList_; }
	GRtm& rtm() { return rtm_; }

public:
	void init();
	void clear();

	static GNetInfo& instance() {
		static GNetInfo netInfo;
		return netInfo;
	}
};
