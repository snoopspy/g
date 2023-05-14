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
#include "net/manage/ghostmgr.h"
#include "net/manage/ghostscan.h"

// ----------------------------------------------------------------------------
// GAutoArpSpoof
// ----------------------------------------------------------------------------
struct GAutoArpSpoof : GArpSpoof, GHostMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(ulong floodingTimeout MEMBER floodingTimeout_)
	Q_PROPERTY(ulong floodingSendInterval MEMBER floodingSendInterval_)
	Q_PROPERTY(ulong recoverTimeout MEMBER recoverTimeout_)
	Q_PROPERTY(GObjRef hostMgr READ getHostMgr)
	Q_PROPERTY(GObjRef hostScan READ getHostScan)

	GObjRef getHostMgr() { return hostMgr_; }
	GObjRef getHostScan() { return hostScan_; }

public:
	GDuration floodingTimeout_{1000}; // 1 sec
	GDuration floodingSendInterval_{100}; // 100 msec
	GDuration recoverTimeout_{60000}; // 60 sec
	GHostMgr hostMgr_{this};
	GHostScan hostScan_{this};

public:
	Q_INVOKABLE GAutoArpSpoof(QObject* parent = nullptr);
	~GAutoArpSpoof() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// GHostMgr::Managable
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostChanged(GMac mac, GHostMgr::HostValue* hostValue) override;

protected:
	GIp gwIp_;
	GMac gwMac_;

	typedef QPair<GFlow::IpFlowKey, GFlow::IpFlowKey> TwoFlowKey;
	struct FloodingThread : QThread {
		FloodingThread(GAutoArpSpoof* parent, TwoFlowKey twoFlowKey, GEthArpPacket infectPacket1, GEthArpPacket infectPacket2);
		~FloodingThread() override;
		void run() override;

		GAutoArpSpoof* parent_;
		GStateWaitEvent swe_;
		TwoFlowKey twoFlowKey_;
		GEthArpPacket infectPacket_[2];
	};
	struct FloodingThreadMap : QMap<TwoFlowKey, FloodingThread*> {
		QRecursiveMutex m_;
	} floodingThreadMap_;

	struct RecoverThread : QThread {
		RecoverThread(GAutoArpSpoof* parent, TwoFlowKey twoFlowKey, Flow flow1, Flow flow2);
		~RecoverThread() override;
		void run() override;

		GAutoArpSpoof* parent_;
		GStateWaitEvent swe_;
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
