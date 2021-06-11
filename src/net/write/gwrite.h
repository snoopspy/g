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

#include "base/gstateobj.h"
#include "net/packet/gpacket.h"

// ----------------------------------------------------------------------------
// GWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GWrite : GStateObj {
	Q_OBJECT

public:
	Q_INVOKABLE GWrite(QObject* parent = nullptr) : GStateObj(parent) {}
	~GWrite() override {}

	GPacket::Dlt dlt() { return dlt_; }

protected:
	GPacket::Dlt dlt_{GPacket::Null};

public:
	virtual GPacket::Result write(GBuf buf);
	virtual GPacket::Result write(GPacket* packet);

signals:
	void written(GPacket* packet);
};

