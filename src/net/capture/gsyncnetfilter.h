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

#include "gnetfilter.h"

// ----------------------------------------------------------------------------
// GSyncNetFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GSyncNetFilter : GNetFilter {
	Q_OBJECT

public:
	Q_INVOKABLE GSyncNetFilter(QObject* parent = nullptr) : GNetFilter(parent) {
		autoRead_ = false;
	}
};
