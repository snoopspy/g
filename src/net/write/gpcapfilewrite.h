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

#include "gpcapwrite.h"

// ----------------------------------------------------------------------------
// GPcapFileWrite
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapFileWrite : GPcapWrite {
	Q_OBJECT
	Q_PROPERTY(GPacket::Dlt dlt MEMBER dlt_)
	Q_PROPERTY(int snapLen MEMBER snapLen_)
	Q_PROPERTY(QString fileName MEMBER fileName_)
	Q_PROPERTY(bool resolveFileNameByTime MEMBER resolveFileNameByTime_)

public:
	int snapLen_{GPacket::MaxBufSize};
	QString fileName_{"pcap/yyMMdd-hhmmss-zzz.'pcap'"};
	bool resolveFileNameByTime_{true};

public:
	Q_INVOKABLE GPcapFileWrite(QObject* parent = nullptr) : GPcapWrite(parent) {
		dlt_ = GPacket::Eth;
	}
	~GPcapFileWrite() override { close(); }

	void setDlt(GPacket::Dlt dlt) { dlt_ = dlt; }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	pcap_dumper_t* pcap_dumper_{nullptr};

public:
	GPacket::Result write(GBuf buf) override;

public slots:
	GPacket::Result write(GPacket* packet) override;

signals:
	void written(GPacket* packet);
};
