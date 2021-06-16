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

#include "gcapture.h"
#include <QProcess>

// ----------------------------------------------------------------------------
// GPcapPipe
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapPipe : GCapture {
	Q_OBJECT
	Q_PROPERTY(QString command MEMBER command_)
	Q_PROPERTY(int readTimeout MEMBER readTimeout_)
	Q_PROPERTY(bool removeCr MEMBER removeCr_)
	Q_PROPERTY(int bufSize MEMBER bufSize_)

public:
	QString command_{"adb exec-out su -c \"corepcap dev wlan0 file -\""};
	int readTimeout_{100};
	bool removeCr_{true};
	int bufSize_{GPacket::MaxBufSize};

public:
	Q_INVOKABLE GPcapPipe(QObject* parent = nullptr);
	~GPcapPipe() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;

	GPacket::Dlt dlt() override { return dlt_; }
	PathType pathType() override { return OutOfPath; }

protected:
	bool removeCrLastBytesBuffered_;
	char removeCrLastBytes_;
	GPacket::Dlt dlt_{GPacket::Null};
	QProcess* process_{nullptr};
	gbyte* recvBuf_{nullptr};
	qint64 recvAll(char *data, size_t size);
};
