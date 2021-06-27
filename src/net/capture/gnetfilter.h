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

#include "gvirtualnetfilter.h"
#include "base/other/gcommand.h"

#ifdef Q_OS_LINUX

// ----------------------------------------------------------------------------
// GNetFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GNetFilter : GVirtualNetFilter {
	Q_OBJECT
	Q_PROPERTY(int bufSize MEMBER bufSize_)
	Q_PROPERTY(GObjRef command READ getCommand)

public:
	GObjRef getCommand() { return &command_; }

public:
	int bufSize_{GPacket::MaxBufSize};
	GCommand command_;

public:
	Q_INVOKABLE GNetFilter(QObject* parent = nullptr);
	~GNetFilter() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	GPacket::Result relay(GPacket* packet) override;
	GPacket::Result drop(GPacket* packet) override;

protected:
	struct nfq_handle* h_{nullptr};
	struct nfq_q_handle* qh_{nullptr};
	int fd_{0};

	gbyte* recvBuf_{nullptr};
	nfq_callback* cb_;

private:
	uint32_t id_;
	GIpPacket* ipPacket_;
	static int _callback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfad, void* data);
};

#endif // Q_OS_LINUX
