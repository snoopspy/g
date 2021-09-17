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
// GRemotePcapDeviceWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GRemotePcapDeviceWrite : GPcapDeviceWrite {
	Q_OBJECT
	Q_PROPERTY(QString ip MEMBER ip_)
	Q_PROPERTY(quint16 port MEMBER port_)

public:
	QString ip_{"127.0.0.1"};
	quint16 port_{GDemon::DefaultPort};

public:
	Q_INVOKABLE GRemotePcapDeviceWrite(QObject* parent = nullptr);
	~GRemotePcapDeviceWrite() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result write(GBuf buf) override;

public slots:
	GPacket::Result write(GPacket* packet) override;

protected:
	GDemonClient* demonClient_{nullptr};

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
