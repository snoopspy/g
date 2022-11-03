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
	Q_PROPERTY(bool checkFcsSize MEMBER checkFcsSize_)

public:
	bool checkFcsSize_{true};

public:
	Q_INVOKABLE GMonitorDeviceWrite(QObject* parent = nullptr);
	~GMonitorDeviceWrite() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result write(GBuf buf) override;

public slots:
	GPacket::Result write(GPacket* packet) override;
};
