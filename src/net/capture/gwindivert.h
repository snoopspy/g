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

#ifdef Q_OS_WIN

#include "net/_win/windivert.h"

// ----------------------------------------------------------------------------
// GWinDivert
// ----------------------------------------------------------------------------
struct G_EXPORT GWinDivert : GCapture {
	Q_OBJECT
	Q_PROPERTY(QString filter MEMBER filter_)
	Q_PROPERTY(qint16 priority MEMBER priority_)
	Q_PROPERTY(int layer MEMBER layer_)
	Q_PROPERTY(quint64 flags MEMBER flags_)
	Q_PROPERTY(quint64 queueLen MEMBER queueLen_)
	Q_PROPERTY(quint64 queueTime MEMBER queueTime_)
	Q_PROPERTY(int tos MEMBER tos_)
	Q_PROPERTY(bool correctIpChecksum MEMBER correctIpChecksum_)
	Q_PROPERTY(int bufSize MEMBER bufSize_)

public:
	QString filter_{"true"};
	int16_t priority_{0};
	int layer_{WINDIVERT_LAYER_NETWORK};
	UINT64 flags_{0};
	UINT64 queueLen_{8192};
	UINT64 queueTime_{1024};
	int tos_{0};
	bool correctIpChecksum_{true};
	int bufSize_{GPacket::MaxBufSize};

public:
	Q_INVOKABLE GWinDivert(QObject* parent = nullptr);
	~GWinDivert() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	HANDLE handle_{nullptr};
	gbyte* pktBuf_{nullptr};
	WINDIVERT_ADDRESS windivertAddress_;

public:
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	GPacket::Result relay(GPacket* packet) override;

	GPacket::Dlt dlt() override { return GPacket::Ip; }
	PathType pathType() override { return ((flags_ & WINDIVERT_FLAG_SNIFF) == 0) ? InPath : OutOfPath; }
};

#endif // Q_OS_WIN
