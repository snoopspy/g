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

#include "gpcappipenexmon.h"

// ----------------------------------------------------------------------------
// GSyncPcapPipeNexmon
// ----------------------------------------------------------------------------
struct G_EXPORT GSyncPcapPipeNexmon : GPcapPipeNexmon {
	Q_OBJECT

public:
	Q_INVOKABLE GSyncPcapPipeNexmon(QObject* parent = nullptr) : GPcapPipeNexmon(parent) {
		autoRead_ = false;
	}
};
