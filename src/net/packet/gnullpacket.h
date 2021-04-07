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

#include "gpacket.h"

// ----------------------------------------------------------------------------
// GNullPacket
// ----------------------------------------------------------------------------
struct G_EXPORT GNullPacket : GPacket {
	GNullPacket(QObject* parent = nullptr) : GPacket(parent) {
		dataLinkType_ = GPacket::Null;
	}

	void parse() override;
};
typedef GNullPacket *PNullPacket;
