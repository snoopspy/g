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

#include "base/gobj.h"

// ----------------------------------------------------------------------------
// GLog
// ----------------------------------------------------------------------------
struct G_EXPORT GLog : GObj {
	GLog(QObject* parent = nullptr) : GObj(parent) {}
	~GLog() override {}
	virtual void write(QString& msg) = 0;
};
typedef GLog *PLog;
