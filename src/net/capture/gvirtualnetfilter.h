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
#include "net/packet/gippacket.h"

#ifdef Q_OS_LINUX

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

// ----------------------------------------------------------------------------
// GVirtualNetFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GVirtualNetFilter : GCapture {
	Q_OBJECT
	Q_PROPERTY(quint16 queueNum MEMBER queueNum_)
	Q_PROPERTY(Verdict acceptVerdict MEMBER acceptVerdict_)
	Q_PROPERTY(quint32 mark MEMBER mark_)
	Q_PROPERTY(int waitTimeout MEMBER waitTimeout_)

	Q_ENUMS(Verdict)

public:
	enum Verdict {
		ACCEPT = 1,
		REPEAT = 4
	};

	uint16_t queueNum_{0};
	Verdict acceptVerdict_{ACCEPT};
	uint32_t mark_{0};
	int waitTimeout_{1}; // 1 msec

public:
	Q_INVOKABLE GVirtualNetFilter(QObject* parent = nullptr) : GCapture(parent) {}
	~GVirtualNetFilter() override {}

public:
	GPacket::Dlt dlt() override { return GPacket::Ip; }
	PathType pathType() override { return InPath; }
};

#endif // Q_OS_LINUX
