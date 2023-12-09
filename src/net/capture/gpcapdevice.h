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
#ifdef Q_OS_ANDROID
#include "net/demon/gdemonclient.h"
#endif

// ----------------------------------------------------------------------------
// GPcapDevice
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapDevice : GPcapCapture {
	Q_OBJECT
	Q_PROPERTY(QString intfName MEMBER intfName_)
	Q_PROPERTY(int snapLen MEMBER snapLen_)
	Q_PROPERTY(int flags MEMBER flags_)
	Q_PROPERTY(int readTimeout MEMBER readTimeout_)
	Q_PROPERTY(int waitTimeout MEMBER waitTimeout_)
	Q_PROPERTY(bool nonBlock MEMBER nonBlock_)

public:
	QString intfName_;
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
	bool nonBlock_{false};

public:
	Q_INVOKABLE GPcapDevice(QObject* parent = nullptr);
	~GPcapDevice() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GIntf* intf() { return intf_; }

protected:
	GIntf* intf_{nullptr};

public:
#ifdef Q_OS_ANDROID
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	GPacket::Result relay(GPacket* packet) override;
#else
	GPacket::Result read(GPacket* packet) override;
#endif

#ifdef Q_OS_ANDROID
public:
	int port_{GDemon::DefaultPort};
	GDemonClient* demonClient_{nullptr};
#endif

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
