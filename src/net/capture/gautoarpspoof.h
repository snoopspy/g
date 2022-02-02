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
#include "net/host/ghostdetect.h"
#include "net/host/ghostdelete.h"
#include "net/host/ghostscan.h"

// ----------------------------------------------------------------------------
// GAutoArpSpoof
// ----------------------------------------------------------------------------
struct GAutoArpSpoof : GArpSpoof {
	Q_OBJECT
	Q_PROPERTY(ulong floodingTimeout MEMBER floodingTimeout_)
	Q_PROPERTY(ulong floodingSendInterval MEMBER floodingSendInterval_)
	Q_PROPERTY(ulong recoverTimeout MEMBER recoverTimeout_)
	Q_PROPERTY(GObjRef hostDetect READ getHostDetect)
	Q_PROPERTY(GObjRef hostDelete READ getHostDelete)
	Q_PROPERTY(GObjRef hostScan READ getHostScan)

	GObjRef getHostDetect() { return hostDetect_; }
	GObjRef getHostDelete() { return hostDelete_; }
	GObjRef getHostScan() { return hostScan_; }

public:
	GDuration floodingTimeout_{1000}; // 1 sec
	GDuration floodingSendInterval_{100}; // 100 msec
	GDuration recoverTimeout_{60000}; // 60 sec
	GHostDetect hostDetect_{this};
	GHostDelete hostDelete_{this};
	GHostScan hostScan_{this};

public:
	Q_INVOKABLE GAutoArpSpoof(QObject* parent = nullptr);
	~GAutoArpSpoof() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public slots:
	void processHostDetected(GHostDetect::Host* host);

protected:
	GIp gwIp_;
	GMac gwMac_;

	typedef QPair<GFlow::IpFlowKey, GFlow::IpFlowKey> TwoFlowKey;
	struct FloodingThread : QThread {
		FloodingThread(GAutoArpSpoof* parent, TwoFlowKey twoFlowKey, GEthArpHdr infectPacket1, GEthArpHdr infectPacket2);
		~FloodingThread() override;
		void run() override;

		GAutoArpSpoof* parent_;
		GWaitEvent we_;
		TwoFlowKey twoFlowKey_;
		GEthArpHdr infectPacket_[2];
	};
	struct FloodingThreadMap : QMap<TwoFlowKey, FloodingThread*> {
		QRecursiveMutex m_;
	} floodingThreadMap_;

	struct RecoverThread : QThread {
		RecoverThread(GAutoArpSpoof* parent, TwoFlowKey twoFlowKey, Flow flow1, Flow flow2);
		~RecoverThread() override;
		void run() override;

		GAutoArpSpoof* parent_;
		GWaitEvent we_;
		TwoFlowKey twoFlowKey_;
		Flow flow1_, flow2_;
	};
	struct RecoverThreadMap : QMap<TwoFlowKey, RecoverThread*> {
		QRecursiveMutex m_;
	} recoverThreadMap_;

	void removeFlows(Flow* flow1, Flow* flow2);

public:
	void removeFlows(GIp senderIp1, GIp targetIp1, GIp senderIp2, GIp targetIp2);
};
