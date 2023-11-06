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
	Q_PROPERTY(bool checkRadioInfo MEMBER checkRadioInfo_)

public:
	bool checkRadioInfo_{true};

public:
	Q_INVOKABLE GMonitorDevice(QObject* parent = nullptr);
	~GMonitorDevice() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	struct {
		uint64_t radioHdr64_; // GRadioHdr;
		uint32_t fcsSize_;
	} radioInfo_;

protected:
	bool getRadioInfoFromFile();
	bool getRadioInfoFromDevice();
};
