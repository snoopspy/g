// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#include "base/gstateobj.h"
#include "net/packet/gippacket.h"

// ----------------------------------------------------------------------------
// GConvertIp
// ----------------------------------------------------------------------------
struct G_EXPORT GConvertIp : GStateObj {
	Q_OBJECT

public:
	Q_INVOKABLE GConvertIp(QObject* parent = nullptr) : GStateObj(parent) {}
	~GConvertIp() override {}

protected:
	bool doOpen() override { return true; }
	bool doClose() override { return true; }

protected:
	GIpPacket convertedIpPacket_;
	gbyte convertedIpBuf_[GPacket::MaxBufSize];

public slots:
	void convert(GPacket* packet);

signals:
	void converted(GPacket* packet);
};
