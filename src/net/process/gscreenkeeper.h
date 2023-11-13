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

// ----------------------------------------------------------------------------
// GScreenKeeper
// ----------------------------------------------------------------------------
struct G_EXPORT GScreenKeeper : GStateObj {
	Q_OBJECT

public:
	Q_INVOKABLE GScreenKeeper(QObject* parent = nullptr) : GStateObj(parent) {}
	~GScreenKeeper() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	bool keepScreen(bool on);
};
