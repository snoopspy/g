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
#include "net/gwritable.h"

// ----------------------------------------------------------------------------
// GWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GWrite : GStateObj, GWritable {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)

public:
	bool enabled_{true};

public:
	Q_INVOKABLE GWrite(QObject* parent = nullptr) : GStateObj(parent) {}
	~GWrite() override {}

	GPacket::Dlt dlt() { return dlt_; }

protected:
	GPacket::Dlt dlt_{GPacket::Null};

public:
	GPacket::Result writeBuf(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;

signals:
	void written(GPacket* packet);
};
