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
// GCorrectChecksum
// ----------------------------------------------------------------------------
struct G_EXPORT GCorrectChecksum : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool ipChecksum MEMBER ipChecksum_)
	Q_PROPERTY(bool tcpChecksum MEMBER tcpChecksum_)
	Q_PROPERTY(bool udpChecksum MEMBER udpChecksum_)
	Q_PROPERTY(bool icmpChecksum MEMBER icmpChecksum_)

public:
	bool ipChecksum_{true};
	bool tcpChecksum_{true};
	bool udpChecksum_{true};
	bool icmpChecksum_{true};

public:
	Q_INVOKABLE GCorrectChecksum(QObject* parent = nullptr) : GStateObj(parent) {}
	~GCorrectChecksum() override {}

protected:
	bool doOpen() override { return true; }
	bool doClose() override { return true; }

public slots:
	void correct(GPacket* packet);

signals:
	void corrected(GPacket* packet);
};
