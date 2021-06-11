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

#include "gpcapcapture.h"
#include "net/gnetinfo.h"

// ----------------------------------------------------------------------------
// GVirtualPcapDevice
// ----------------------------------------------------------------------------
struct G_EXPORT GVirtualPcapDevice : GPcapCapture {
	Q_OBJECT
	Q_PROPERTY(QString intfName MEMBER intfName_)
	Q_PROPERTY(int snapLen MEMBER snapLen_)
	Q_PROPERTY(int flags MEMBER flags_)
	Q_PROPERTY(int readTimeout MEMBER readTimeout_)
	Q_PROPERTY(int waitTimeout MEMBER waitTimeout_)

public:
	QString intfName_{""};
	int snapLen_{GPacket::MaxBufSize};
	int flags_{1}; // PCAP_OPENFLAG_PROMISCUOUS
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	int readTimeout_{-1}; // -1 msec
	int waitTimeout_{1}; // 1 msec
#endif // Q_OS_LINUX
#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
	int readTimeout_{1}; // 1 msec
	int waitTimeout_{0}; // 0 msec
#endif

public:
	Q_INVOKABLE GVirtualPcapDevice(QObject* parent = nullptr) : GPcapCapture(parent) {}
	~GVirtualPcapDevice() override {}

public:
	GInterface* intf() { return intf_; }

protected:
	GInterface* intf_{nullptr};
};
