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

#include "base/gstateobj.h"
#include "base/sys/gthread.h"
#include "net/greadable.h"
#include "net/gwritable.h"
#include "net/packet/ganypacket.h"

// ----------------------------------------------------------------------------
// GCapture
// ----------------------------------------------------------------------------
struct G_EXPORT GCapture : GStateObj, GReadable, GWritable {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(bool autoParse MEMBER autoParse_)

public:
	bool enabled_{true};
	bool autoParse_{true};

protected:
	bool autoRead_{true};

public:
	enum PathType {
		InPath,
		OutOfPath
	};

public:
	GCapture(QObject* parent = nullptr) : GStateObj(parent) {}
	~GCapture() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result writeBuf(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	virtual GPacket::Result relay(GPacket* packet);
	virtual GPacket::Result drop(GPacket* packet);

	virtual GPacket::Dlt dlt() { return GPacket::Null; }
	virtual PathType pathType() { return OutOfPath; }

protected:
	virtual void captureRun();

	GAnyPacket anyPacket_;
	struct CaptureThread : GThread {
		CaptureThread(GCapture* capture) : GThread(capture) {}
	protected:
		void run() override;
	} captureThread_{this};

signals:
	void captured(GPacket* packet);
};
