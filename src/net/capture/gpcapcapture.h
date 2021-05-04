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

#include <pcap.h>
#include "gcapture.h"

// ----------------------------------------------------------------------------
// GPcapCapture
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapCapture : GCapture {
	Q_OBJECT
	Q_PROPERTY(QString filter MEMBER filter_)

public:
	QString filter_{""};

public:
	GPcapCapture(QObject* parent = nullptr) : GCapture(parent) {}
	~GPcapCapture() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	GPacket::Result relay(GPacket* packet) override;

	GPacket::Dlt dlt() override { return dlt_; }
	PathType pathType() override { return OutOfPath; }

protected:
	bool pcapProcessFilter(pcap_if_t* dev);

	pcap_t* pcap_{nullptr};
	GPacket::Dlt dlt_{GPacket::Null};
};
