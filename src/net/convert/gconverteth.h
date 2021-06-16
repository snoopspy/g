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
#include "net/packet/gethpacket.h"

// ----------------------------------------------------------------------------
// GConvertEth
// ----------------------------------------------------------------------------
struct G_EXPORT GConvertEth : GStateObj {
	Q_OBJECT
	Q_PROPERTY(QString smac READ getSmac WRITE setSmac)
	Q_PROPERTY(QString dmac READ getDmac WRITE setDmac)
	Q_PROPERTY(quint16 type MEMBER type_)
	Q_PROPERTY(int bufSize MEMBER bufSize_)

	QString getSmac() { return QString(smac_); }
	void setSmac(QString value) { smac_ = value; }
	QString getDmac() { return QString(dmac_); }
	void setDmac(QString value) { dmac_ = value; }

public:
	GMac smac_{GMac::nullMac()};
	GMac dmac_{GMac::nullMac()};
	uint16_t type_{GEthHdr::Ip4};
	int bufSize_{GPacket::MaxBufSize};

public:
	Q_INVOKABLE GConvertEth(QObject* parent = nullptr) : GStateObj(parent) {}
	~GConvertEth() override {}

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GEthPacket convertedEthPacket_;
	gbyte* convertedEthBuf_{nullptr};

public slots:
	void convert(GPacket* packet);

signals:
	void converted(GPacket* packet);
};
