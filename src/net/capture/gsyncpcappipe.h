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

#include "gpcappipe.h"

// ----------------------------------------------------------------------------
// GSyncPcapPipe
// ----------------------------------------------------------------------------
struct G_EXPORT GSyncPcapPipe : GPcapPipe {
	Q_OBJECT

public:
	Q_INVOKABLE GSyncPcapPipe(QObject* parent = nullptr) : GPcapPipe(parent) {
		autoRead_ = false;
	}
};
