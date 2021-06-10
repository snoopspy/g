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

#include "garpspoof.h"

// ----------------------------------------------------------------------------
// GAutoArpSpoof
// ----------------------------------------------------------------------------
struct GAutoArpSpoof : GArpSpoof {
	Q_OBJECT

public:
	Q_INVOKABLE GAutoArpSpoof(QObject* parent = nullptr) : GArpSpoof(parent) {}
	~GAutoArpSpoof() override {}

protected:
	void processArp(GPacket* packet) override;
};
