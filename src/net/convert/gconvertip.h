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
	Q_PROPERTY(int bufSize MEMBER bufSize_)

public:
	int bufSize_{GPacket::MaxBufSize};

public:
	Q_INVOKABLE GConvertIp(QObject* parent = nullptr) : GStateObj(parent) {}
	~GConvertIp() override {}

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GIpPacket convertedIpPacket_;
	gbyte* convertedIpBuf_{nullptr};

public slots:
	void convert(GPacket* packet);

signals:
	void converted(GPacket* packet);
};
