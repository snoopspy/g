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

#ifdef Q_OS_ANDROID_GILGIL
	#include "net/capture/gsyncremotepcapdevice.h"
	typedef GRemotePcapDevice GAtmScanDevice;
#else // Q_OS_ANDROID_GILGIL
	#include "net/capture/gsyncpcapdevice.h"
	typedef GPcapDevice GAtmScanDevice;
#endif // Q_OS_ANDROID_GILGIL


#include "net/packet/gpacket.h"
#include "net/gnetinfo.h"
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
struct G_EXPORT GAtm : GAtmMap {
	bool allResolved();
	void deleteUnresolved();
	bool wait(GAtmScanDevice* device, GDuration timeout = G::Timeout);

protected:
	bool sendQueries(GAtmScanDevice* device, GInterface* intf);

protected:
	// --------------------------------------------------------------------------
	// SendThread
	// --------------------------------------------------------------------------
	struct SendThread : QThread {
		SendThread(GAtm* resolve, GAtmScanDevice* device, GInterface* intf, GDuration timeout);
		GAtm* atm_;
		GAtmScanDevice* device_;
		GInterface* intf_;
		GDuration timeout_;
		GWaitEvent we_;
	protected:
		void run() override;
	};
};

