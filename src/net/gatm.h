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
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GAtm(Arp Table Manager)
// ----------------------------------------------------------------------------
typedef QMap<GIp, GMac> GAtmMap;

struct G_EXPORT GAtm : public GSyncPcapDevice, GAtmMap {
	Q_INVOKABLE GAtm(QObject* parent = nullptr) : GSyncPcapDevice(parent) {}
	~GAtm() override { GSyncPcapDevice::close(); }

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
		SendThread(GAtm* atm, GDuration timeout) : atm_(atm), timeout_(timeout) {}
		GAtm* atm_;
		GDuration timeout_;
		GStateWaitEvent swe_;
	protected:
		void run() override;
	};
};
