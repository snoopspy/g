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

#include "net/capture/gsyncpcapdevice.h"
#include "net/capture/gsyncremotepcapdevice.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GEthArpHdr
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct GEthArpHdr {
	GEthHdr ethHdr_;
	GArpHdr arpHdr_;
};
#pragma pack(pop)

// ----------------------------------------------------------------------------
// GAtm(Arp Table Manager)
// ----------------------------------------------------------------------------
typedef QMap<GIp, GMac> GAtmMap;

template <typename Base>
struct G_EXPORT GAtm_ : public Base, GAtmMap {
	Q_INVOKABLE GAtm_(QObject* parent = nullptr) : Base(parent) {}
	~GAtm_() override {}

	bool allResolved();
	void deleteUnresolved();
	bool wait(GDuration timeout = G::Timeout);

protected:
	bool sendQueries();

protected:
	// --------------------------------------------------------------------------
	// SendThread
	// --------------------------------------------------------------------------
	struct SendThread : QThread {
		SendThread(GAtm_* atm, GDuration timeout) : atm_(atm), timeout_(timeout) {}
		GAtm_* atm_;
		GDuration timeout_;
		GWaitEvent we_;
	protected:
		void run() override;
	};
};

typedef GAtm_<GSyncPcapDevice> GAtm;
typedef GAtm_<GSyncRemotePcapDevice> GRemoteAtm;
