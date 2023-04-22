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

#include "gpcapdevicewrite.h"

// ----------------------------------------------------------------------------
// GMonitorDeviceWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GMonitorDeviceWrite : GPcapDeviceWrite {
	Q_OBJECT
	Q_PROPERTY(bool clearRadioHdr MEMBER clearRadioHdr_)
	Q_PROPERTY(bool checkFcsSize MEMBER checkFcsSize_)

public:
	bool clearRadioHdr_{true};
	bool checkFcsSize_{true};

public:
	Q_INVOKABLE GMonitorDeviceWrite(QObject* parent = nullptr);
	~GMonitorDeviceWrite() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	// for checkRadioHdr
	gbyte sendBuffer_[GPacket::MaxBufSize];

public:
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
};
