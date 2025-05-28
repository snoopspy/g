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
#include "net/process/gcommand.h"

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

// ----------------------------------------------------------------------------
// GNetFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GNetFilter : GCapture {
	Q_OBJECT
	Q_PROPERTY(quint16 queueNum MEMBER queueNum_)
	Q_PROPERTY(int waitTimeout MEMBER waitTimeout_)
	Q_PROPERTY(bool nonBlock MEMBER nonBlock_)
	Q_PROPERTY(Verdict acceptVerdict MEMBER acceptVerdict_)
	Q_PROPERTY(quint32 mark MEMBER mark_)
	Q_PROPERTY(int bufSize MEMBER bufSize_)
	Q_PROPERTY(GObjRef command READ getCommand)

	Q_ENUMS(Verdict)

public:
	enum Verdict {
		Drop = NF_DROP, // 0
		Accept = NF_ACCEPT, // 1
		Stolen = NF_STOLEN, // 2
		Queue = NF_QUEUE, // 3
		Repeat = NF_REPEAT, // 4
		Stop = NF_STOP // 5
	};

	uint16_t queueNum_{0};
	int waitTimeout_{1}; // 1 msec
	bool nonBlock_{true};
	Verdict acceptVerdict_{Accept};
	uint32_t mark_{0};
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
	GPacket::Result writeBuf(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	GPacket::Result relay(GPacket* packet) override;
	GPacket::Result drop(GPacket* packet) override;

	GPacket::Dlt dlt() override { return GPacket::Ip; }
	PathType pathType() override { return InPath; }

protected:
	struct nfq_handle* h_{nullptr};
	struct nfq_q_handle* qh_{nullptr};
	int fd_{0};

	gbyte* recvBuf_{nullptr};

#ifdef Q_OS_ANDROID
private:
	uint32_t id_;
	GIpPacket* ipPacket_;
public:
	GDemonClient* demonClient_{nullptr};
#else

private:
	struct Pkt {
		struct timeval ts_;
		GBuf buf_;
		uint32_t id_;
	};
	struct Pkts : QList<Pkt> {} pkts_;
protected:
	nfq_callback* cb_;

private:
	static int _callback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfad, void* data);
#endif
};
