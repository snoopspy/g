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

#include "gvirtualpcapdevicewrite.h"

// ----------------------------------------------------------------------------
// GPcapDeviceWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapDeviceWrite : GVirtualPcapDeviceWrite {
	Q_OBJECT

public:
	Q_INVOKABLE GPcapDeviceWrite(QObject* parent = nullptr);
	~GPcapDeviceWrite() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result write(GBuf buf) override;

public slots:
	GPacket::Result write(GPacket* packet) override;

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
