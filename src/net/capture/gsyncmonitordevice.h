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

#include "gmonitordevice.h"

// ----------------------------------------------------------------------------
// GSyncMonitorDevice
// ----------------------------------------------------------------------------
struct G_EXPORT GSyncMonitorDevice : GMonitorDevice {
	Q_OBJECT

public:
	Q_INVOKABLE GSyncMonitorDevice(QObject* parent = nullptr) : GMonitorDevice(parent) {
		autoRead_ = false;
	}
};
