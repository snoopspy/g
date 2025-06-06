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
#ifdef Q_OS_ANDROID
#include "net/demon/gdemonclient.h"
#endif

// ----------------------------------------------------------------------------
// GPcapDeviceWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapDeviceWrite : GPcapWrite {
	Q_OBJECT
	Q_PROPERTY(QString intfName MEMBER intfName_)
	Q_PROPERTY(int mtu MEMBER mtu_)

public:
	QString intfName_;
	int mtu_{GPacket::MtuSize};

public:
	Q_INVOKABLE GPcapDeviceWrite(QObject* parent = nullptr);
	~GPcapDeviceWrite() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GIntf* intf() { return intf_; }

protected:
	GIntf* intf_{nullptr};

public:
	GPacket::Result writeBuf(GBuf buf) override;

public slots:
	GPacket::Result write(GPacket* packet) override;

#ifdef Q_OS_ANDROID
protected:
	int port_{GDemon::DefaultPort};
	GDemonClient* demonClient_{nullptr};
#endif

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
