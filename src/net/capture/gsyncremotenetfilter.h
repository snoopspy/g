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

#include "gremotenetfilter.h"

// ----------------------------------------------------------------------------
// GSyncRemoteNetFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GSyncRemoteNetFilter : GRemoteNetFilter {
	Q_OBJECT

public:
	Q_INVOKABLE GSyncRemoteNetFilter(QObject* parent = nullptr) : GRemoteNetFilter(parent) {
		autoRead_ = false;
	}
};
