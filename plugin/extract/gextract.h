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
// GExtract
// ----------------------------------------------------------------------------
struct G_EXPORT GExtract : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)

public:
	bool enabled_{true};

public:
	Q_INVOKABLE GExtract(QObject* parent = nullptr) : GStateObj(parent) {}
	~GExtract() override {}
};
