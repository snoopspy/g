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

#include "gpcapdevice.h"

// ----------------------------------------------------------------------------
// GMonitorDevice
// ----------------------------------------------------------------------------
struct G_EXPORT GMonitorDevice : GPcapDevice {
	Q_OBJECT

public:
	Q_INVOKABLE GMonitorDevice(QObject* parent = nullptr);
	~GMonitorDevice() override;

protected:
	int16_t radiotapLen_{-1};

protected:
	bool doOpen() override;
	bool doClose() override;
};
