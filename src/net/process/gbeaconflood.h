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

#include "net/capture/gsyncmonitordevice.h"
#include "base/sys/gthread.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GBeaconFlood
// ----------------------------------------------------------------------------
struct G_EXPORT GBeaconFlood : GSyncMonitorDevice {
	Q_OBJECT
	Q_PROPERTY(QStringList messages MEMBER messages_)
	Q_PROPERTY(ulong interval MEMBER interval_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(QString startMac READ getStartMac WRITE setStartMac)

	QString getStartMac() { return QString(startMac_); }
	void setStartMac(QString value) { startMac_ = value; }

public:
	QStringList messages_;
#ifdef Q_OS_ANDROID
	GDuration interval_{0};
	GDuration sendInterval_{5};
#else //
	GDuration interval_{10};
	GDuration sendInterval_{1};
#endif // Q_OS_ANDROID
	GMac startMac_{GMac("00:11:22:33:44:01")};

public:
	Q_INVOKABLE GBeaconFlood(QObject* parent = nullptr);
	~GBeaconFlood() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	struct FloodingThread : GThread {
		FloodingThread(GBeaconFlood* parent) : GThread(parent) {}
		void run() override;

		GWaitEvent we_;
	} floodingThread_{this};

protected:
	struct BeaconFrame {
		GRadioHdr radioHdr_;
		GBeaconHdr beaconHdr_;
		char dummy[256];
		size_t size_;
	};

	struct BeaconFrameList : QList<BeaconFrame> {
	};

	static GMac nextMac(GMac mac);
};
