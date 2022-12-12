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

#include "net/write/gwrite.h"
#ifdef Q_OS_ANDROID
#include "net/demon/gdemonclient.h"
#endif

// ----------------------------------------------------------------------------
// GRawIpSocketWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GRawIpSocketWrite : GWrite {
	Q_OBJECT
	Q_PROPERTY(QString intfName MEMBER intfName_)
	Q_PROPERTY(int mtu MEMBER mtu_)

public:
	QString intfName_;
	int mtu_{GPacket::MtuSize};

public:
	Q_INVOKABLE GRawIpSocketWrite(QObject* parent = nullptr);
	~GRawIpSocketWrite();

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	int sd_{0};

public:
	GPacket::Result write(GBuf buf) override;

public slots:
	GPacket::Result write(GPacket* packet) override;

#ifdef Q_OS_ANDROID
protected:
	GDemonClient* demonClient_{nullptr};
#endif

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
