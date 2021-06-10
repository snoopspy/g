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

#include "gsyncpcapdevice.h"
#include "gsyncremotepcapdevice.h"
#ifdef Q_OS_ANDROID_GILGIL
	typedef GRemotePcapDevice GArpSpoofBaseDevice;
#else // Q_OS_ANDROID_GILGIL
	typedef GPcapDevice GArpSpoofBaseDevice;
#endif // Q_OS_ANDROID_GILGIL

#include "net/gatm.h"
#include "net/flow/gflowkey.h"

// ----------------------------------------------------------------------------
// GArpSpoofFlow
// ----------------------------------------------------------------------------
struct GArpSpoofFlow : GObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(QString senderIp READ getSenderIp WRITE setSenderIp)
	Q_PROPERTY(QString senderMac READ getSenderMac WRITE setSenderMac)
	Q_PROPERTY(QString targetIp READ getTargetIp WRITE setTargetIp)
	Q_PROPERTY(QString targetMac READ getTargetMac WRITE setTargetMac)

	QString getSenderIp() { return QString(senderIp_); }
	void setSenderIp(QString value) { senderIp_ = value; }
	QString getSenderMac() { return QString(senderMac_); }
	void setSenderMac(QString value) { senderMac_ = value; }
	QString getTargetIp() { return QString(targetIp_); }
	void setTargetIp(QString value) { targetIp_ = value; }
	QString getTargetMac() { return QString(targetMac_); }
	void setTargetMac(QString value) { targetMac_ = value; }

public:
	bool enabled_{true};
	GIp senderIp_{0};
	GMac senderMac_{GMac::nullMac()};
	GIp targetIp_{0};
	GMac targetMac_{GMac::nullMac()};
};
typedef GArpSpoofFlow *PArpSpoofFlow;

// ----------------------------------------------------------------------------
// GArpSpoof
// ----------------------------------------------------------------------------
struct G_EXPORT GArpSpoof : GArpSpoofBaseDevice {
	Q_OBJECT
	Q_PROPERTY(QString virtualMac READ getVirtualMac WRITE setVirtualMac)
	Q_PROPERTY(ulong infectInterval MEMBER infectInterval_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(GObjRefArrayPtr flows READ getFlows)

	GObjRefArrayPtr getFlows() { return &flows_; }
	QString getVirtualMac() { return QString(virtualMac_); }
	void setVirtualMac(QString value) { virtualMac_ = value; }

public:
	Q_INVOKABLE GArpSpoof(QObject* parent = nullptr);
	~GArpSpoof() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result read(GPacket* packet) override;
	GPacket::Result write(GBuf buf) override;
	GPacket::Result write(GPacket* packet) override;
	GPacket::Result relay(GPacket* packet) override;

	PathType pathType() override { return InPath; }

public:
	GMac virtualMac_{GMac::nullMac()};
	GDuration infectInterval_{1000};
	GDuration sendInterval_{1};
	GObjRefArray<GArpSpoofFlow> flows_; // for property
	GAtm atm_;

protected:
	struct Flow {
		GIp senderIp_;
		GMac senderMac_;
		GIp targetIp_;
		GMac targetMac_;

		GEthArpHdr infectPacket_;
		GEthArpHdr recoverPacket_;

		Flow() {}
		Flow(GIp senderIp, GMac senderMac, GIp targetIp, GMac targetMac);
		void makePacket(GEthArpHdr* packet, GMac myMac, bool infect);
	};
	QList<Flow> flowList_; // for arp infect and recover
	typedef QMap<GFlow::IpFlowKey, Flow> FlowMap;
	FlowMap flowMap_; // for relay

	GMac myMac_{GMac::nullMac()};

	struct InfectThread : GThread {
		InfectThread(GArpSpoof* arpSpoof) : GThread(arpSpoof), arpSpoof_(arpSpoof) {}
		void run() override;
		GWaitEvent we_;
		GArpSpoof* arpSpoof_;
	} infectThread_{this};

	bool sendArpInfectAll();
	bool sendArpInfect(Flow* flow);
	bool sendARPReciverAll();
	bool sendArpRecover(Flow* flow);

protected:
	virtual void processArp(GPacket* packet); // for GAutoArpSpoof
};
