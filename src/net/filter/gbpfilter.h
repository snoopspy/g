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

#include "base/gstateobj.h"
#include "net/packet/gpacket.h"

// ----------------------------------------------------------------------------
// GBpFilter
// ----------------------------------------------------------------------------
struct G_EXPORT GBpFilter : GStateObj {
	Q_OBJECT
	Q_PROPERTY(GPacket::Dlt dlt MEMBER dlt_)
	Q_PROPERTY(QString filter MEMBER filter_)

public:
	GPacket::Dlt dlt_{GPacket::Eth};
	QString filter_;

public:
	Q_INVOKABLE GBpFilter(QObject* parent = nullptr) : GStateObj(parent) {}
	~GBpFilter() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	pcap_t* pcap_{nullptr};
	bpf_program* code_{nullptr};

public:
	bool filterBuf(GBuf buf);

public slots:
	bool filter(GPacket* packet);

signals:
	void filtered(GPacket* packet);
	void notFiltered(GPacket* packet);
};
