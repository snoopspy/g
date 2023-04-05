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

#include "glog.h"

#include <unistd.h>
#ifdef WIN32
#include <winsock2.h>
#endif // WIN32
#ifdef __linux__
#include <arpa/inet.h>
#endif // __linux__

// ----------------------------------------------------------------------------
// GLogUdp
// ----------------------------------------------------------------------------
struct G_EXPORT GLogUdp : GLog {
	Q_OBJECT
	Q_PROPERTY(QString ip MEMBER ip_)
	Q_PROPERTY(quint16 port MEMBER port_)

public:
	static const quint16 Port = 8908;
	QString ip_{"127.0.0.1"};
	quint16 port_{Port};

public:
	Q_INVOKABLE GLogUdp(QObject* parent = nullptr);
	~GLogUdp() override;

	void write(QString& msg) override;

protected:
	bool configured_{false};
	int sock_{0};
    struct sockaddr_in addr_in_;
	void configure();
};
