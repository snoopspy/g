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
	Q_PROPERTY(ulong floodingInterval MEMBER floodingInterval_)
	Q_PROPERTY(ulong floodingSendInterval MEMBER floodingSendInterval_)

public:
	bool checkIp_{false};
	bool checkArp_{false};
	bool checkDhcp_{true};
	GDuration duration_{60000}; // 1 minute
	GDuration floodingInterval_{5000}; // 5 sec
	GDuration floodingSendInterval_{10}; // 10 msec

public:
	Q_INVOKABLE GAutoArpSpoof(QObject* parent = nullptr);
	~GAutoArpSpoof() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	void processPacket(GPacket* packet) override;

protected:
	GMac myMac_;
	GIp myIp_;
	GMac gwMac_;
	GIp gwIp_;

	bool processIp(GPacket* packet, GMac* mac, GIp* ip);
	bool processArp(GPacket* packet, GMac* mac, GIp* ip);
	bool processDhcp(GPacket* packet, GMac* mac, GIp* ip);

	struct FloodingThread : QThread {
		FloodingThread(GAutoArpSpoof* parent, GEthArpHdr infectPacket1, GEthArpHdr infectPacket2);
		~FloodingThread() override;
		void run() override;
	protected:
		GAutoArpSpoof* parent_;
		GEthArpHdr infectPacket_[2];
	};

signals:
	void floodingNeeded(QByteArray forward, QByteArray backward);

public slots:
	void floodingStart(QByteArray forward, QByteArray backward);
};
