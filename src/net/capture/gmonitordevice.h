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
	Q_PROPERTY(bool checkRadiotapLen MEMBER checkRadiotapLen_)

public:
	bool checkRadiotapLen_{true};

public:
	Q_INVOKABLE GMonitorDevice(QObject* parent = nullptr);
	~GMonitorDevice() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	int16_t getRadiotapLenFromFile();
	int16_t getRadiotapLenFromDevice();
};
