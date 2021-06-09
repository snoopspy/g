// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#include "net/gatm.h"
#include "net/write/gpcapdevicewrite.h"
#include "net/capture/gsyncpcapdevice.h"

// ----------------------------------------------------------------------------
// GConvertEthAutoMac
// ----------------------------------------------------------------------------
struct G_EXPORT GConvertEthAutoMac : GPcapDeviceWrite {
	Q_OBJECT
	Q_PROPERTY(quint16 type MEMBER type_)

public:
	uint16_t type_{GEthHdr::Ip4};

public:
	Q_INVOKABLE GConvertEthAutoMac(QObject* parent = nullptr) : GPcapDeviceWrite(parent) {
		dlt_ = GPacket::Eth;
	}
	~GConvertEthAutoMac() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GMac myMac_{GMac::nullMac()};
	GIp myIp_{0};
	GEthPacket convertedEthPacket_;
	gbyte convertedEthBuf_[GPacket::MaxBufSize];
	GAtm atm_;
	GMac resolveMacByDip(GPacket* packet);

public slots:
	void convert(GPacket* packet);

signals:
	void converted(GPacket* packet);
};
