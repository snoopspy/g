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
	Q_PROPERTY(bool checkIp MEMBER checkIp_)
	Q_PROPERTY(bool checkArp MEMBER checkArp_)
	Q_PROPERTY(bool checkDhcp MEMBER checkDhcp_)
	Q_PROPERTY(ulong duration MEMBER duration_)

public:
	bool checkIp_{false};
	bool checkArp_{false};
	bool checkDhcp_{true};
	GDuration duration_{60000}; // 1 minute

public:
	Q_INVOKABLE GAutoArpSpoof(QObject* parent = nullptr) : GArpSpoof(parent) {}
	~GAutoArpSpoof() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GMac myMac_;
	GIp myIp_;
	GMac gwMac_;
	GIp gwIp_;

	bool processIp(GPacket* packet, GMac* mac, GIp* ip);
	bool processArp(GPacket* packet, GMac* mac, GIp* ip);
	bool processDhcp(GPacket* packet, GMac* mac, GIp* ip);

protected:
	void processPacket(GPacket* packet) override;
};
