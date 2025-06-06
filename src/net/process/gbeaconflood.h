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

#include "net/write/gmonitordevicewrite.h"
#include "base/sys/gthread.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GBeaconFlood
// ----------------------------------------------------------------------------
struct G_EXPORT GBeaconFlood : GMonitorDeviceWrite {
	Q_OBJECT
	Q_PROPERTY(QStringList messages MEMBER messages_)
	Q_PROPERTY(ulong interval MEMBER interval_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(int channel MEMBER channel_)
	Q_PROPERTY(QString startMac READ getStartMac WRITE setStartMac)

	QString getStartMac() { return QString(startMac_); }
	void setStartMac(QString value) { startMac_ = value; }

public:
	QStringList messages_;
	GDuration interval_{10};
	GDuration sendInterval_{1};
	uint8_t channel_{36};
	GMac startMac_{GMac("00:11:22:33:44:00")};

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

		GStateWaitEvent swe_;
	} floodingThread_{this};

protected:
	struct BeaconFrame {
		GRadioHdr radioHdr_;
		GBeaconHdr beaconHdr_;
		char dummy[256];
		size_t size_;
		void build(QString message, int channel, GMac mac);
	};

	struct BeaconFrameList : QList<BeaconFrame> {
	} bfl_;
	QMutex bflMutex_;

public:
	GMac nextMac_;

public:
	void addMessage(QString message);
	void removeMessage(int index);
	static GMac nextMac(GMac mac);
};
