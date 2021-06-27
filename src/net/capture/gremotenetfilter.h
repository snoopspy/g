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
#include "base/other/gremotecommand.h"

#ifdef Q_OS_LINUX

// ----------------------------------------------------------------------------
// GRemoteNetFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GRemoteNetFilter : GVirtualNetFilter {
	Q_OBJECT
	Q_PROPERTY(GObjRef command READ getCommand)
	Q_PROPERTY(QString ip MEMBER ip_)
	Q_PROPERTY(quint16 port MEMBER port_)

public:
	GObjRef getCommand() { return &command_; }
	QString ip_{"127.0.0.1"};
	quint16 port_{GDemon::DefaultPort};

public:
	GRemoteCommand command_;

public:
	Q_INVOKABLE GRemoteNetFilter(QObject* parent = nullptr);
	~GRemoteNetFilter() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	GPacket::Result relay(GPacket* packet) override;
	GPacket::Result drop(GPacket* packet) override;

public:
	GDemonClient* demonClient_{nullptr};

private:
	uint32_t id_;
	GIpPacket* ipPacket_;
};

#endif // Q_OS_LINUX
