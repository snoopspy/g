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

#include "gnetfilter.h"

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

// ----------------------------------------------------------------------------
// GAsyncNetFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GAsyncNetFilter : GNetFilter {
	Q_OBJECT

public:
	Q_INVOKABLE GAsyncNetFilter(QObject* parent = nullptr);
	~GAsyncNetFilter() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	GPacket::Result relay(GPacket* packet) override;

protected:
	void run() override;

private:
	GIpPacket asyncIpPacket_{this};
	static int _asyncCallback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfad, void* data);
};

#endif
