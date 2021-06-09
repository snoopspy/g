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
#ifdef Q_OS_ANDROID_GILGIL
	typedef GSyncRemotePcapDevice GAtmBaseDevice;
#else // Q_OS_ANDROID_GILGIL
	typedef GSyncPcapDevice GAtmBaseDevice;
#endif // Q_OS_ANDROID_GILGIL

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
struct G_EXPORT GAtm : GAtmBaseDevice, GAtmMap {
	Q_INVOKABLE GAtm(QObject* parent = nullptr) : GAtmBaseDevice(parent) {}
	~GAtm() override {}

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
		SendThread(GAtm* atm, GDuration timeout);
		GAtm* atm_;
		GDuration timeout_;
		GWaitEvent we_;
	protected:
		void run() override;
	};
};

