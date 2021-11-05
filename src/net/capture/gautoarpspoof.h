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
	Q_PROPERTY(ulong floodingTimeout MEMBER floodingTimeout_)
	Q_PROPERTY(ulong floodingSendInterval MEMBER floodingSendInterval_)
	Q_PROPERTY(ulong recoverTimeout MEMBER recoverTimeout_)

public:
	bool checkIp_{false};
	bool checkArp_{false};
	bool checkDhcp_{true};
	GDuration floodingTimeout_{5000}; // 5 sec
	GDuration floodingSendInterval_{100}; // 100 msec
	GDuration recoverTimeout_{10000}; // 10 sec

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

		GAutoArpSpoof* parent_;
		GWaitEvent we_;
		GEthArpHdr infectPacket_[2];
	};
	struct FloodingThreadSet : QSet<FloodingThread*> {
		QMutex m_;
	} floodingThreadSet_;

	struct RecoverThread : QThread {
		RecoverThread(GAutoArpSpoof* parent, Flow flow1, Flow flow2);
		~RecoverThread() override;
		void run() override;

		GAutoArpSpoof* parent_;
		GWaitEvent we_;
		Flow flow1_, flow2_;
	};
	struct RecoverThreadSet : QSet<RecoverThread*> {
		QMutex m_;
	} recoverThreadSet_;
};
