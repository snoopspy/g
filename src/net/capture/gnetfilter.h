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
#include "base/other/gcommand.h"

#ifdef Q_OS_LINUX

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

// ----------------------------------------------------------------------------
// GNetFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GNetFilter : GCapture {
	Q_OBJECT
	Q_PROPERTY(quint16 queueNum MEMBER queueNum_)
	Q_PROPERTY(bool nonBlock MEMBER nonBlock_)
	Q_PROPERTY(int waitTimeout MEMBER waitTimeout_)
	Q_PROPERTY(Verdict acceptVerdict MEMBER acceptVerdict_)
	Q_PROPERTY(quint32 mark MEMBER mark_)
	Q_PROPERTY(int bufSize MEMBER bufSize_)
	Q_PROPERTY(GObjRef command READ getCommand)
#ifdef Q_OS_ANDROID
	Q_PROPERTY(QString ip MEMBER ip_)
	Q_PROPERTY(quint16 port MEMBER port_)
#endif

	Q_ENUMS(Verdict)

public:
	enum Verdict {
		ACCEPT = 1, // NF_ACCEPT
		REPEAT = 4 // NF_REPEAT
	};

	uint16_t queueNum_{0};
	bool nonBlock_{true};
	int waitTimeout_{1}; // 1 msec
	Verdict acceptVerdict_{ACCEPT};
	uint32_t mark_{0};
	GObjRef getCommand() { return &command_; }
#ifdef Q_OS_ANDROID
	QString ip_{"127.0.0.1"};
	quint16 port_{GDemon::DefaultPort};
#endif

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

	GPacket::Dlt dlt() override { return GPacket::Ip; }
	PathType pathType() override { return InPath; }

protected:
	struct nfq_handle* h_{nullptr};
	struct nfq_q_handle* qh_{nullptr};
	int fd_{0};

	gbyte* recvBuf_{nullptr};

private:
	uint32_t id_;
	GIpPacket* ipPacket_;

#ifdef Q_OS_ANDROID
public:
	GDemonClient* demonClient_{nullptr};
#else
protected:
	nfq_callback* cb_;

private:
	static int _callback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfad, void* data);
#endif
};

#endif // Q_OS_LINUX
