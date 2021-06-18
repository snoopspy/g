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

#include "gpcapwrite.h"
#include "net/gnetinfo.h"

// ----------------------------------------------------------------------------
// GVirtualPcapDeviceWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GVirtualPcapDeviceWrite : GPcapWrite {
	Q_OBJECT
	Q_PROPERTY(QString intfName MEMBER intfName_)

public:
	QString intfName_{""};

public:
	Q_INVOKABLE GVirtualPcapDeviceWrite(QObject* parent = nullptr) : GPcapWrite(parent) {}
	~GVirtualPcapDeviceWrite() override {}

public:
	GInterface* intf() { return intf_; }

protected:
	GInterface* intf_{nullptr};
};
