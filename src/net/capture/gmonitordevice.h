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
	Q_PROPERTY(bool checkRadioLen MEMBER checkRadioLen_)

public:
	bool checkRadioLen_{true};

public:
	Q_INVOKABLE GMonitorDevice(QObject* parent = nullptr);
	~GMonitorDevice() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	struct {
		int16_t len_;
		uint32_t fcsSize_;
	} radioInfo_;

protected:
	bool getRadioInfoFromFile();
	bool getRadioInfoFromDevice();
};
