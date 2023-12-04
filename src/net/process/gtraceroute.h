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

#include "net/manage/gipmgr.h"
#include "net/capture/gpcapdevice.h"
#include "net/write/grawipsocketwrite.h"
#include "base/sys/gwaitevent.h"

// ----------------------------------------------------------------------------
// GTraceRoute
// ----------------------------------------------------------------------------
struct G_EXPORT GTraceRoute : GStateObj, GIpMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(int maxHop MEMBER maxHop_)
	Q_PROPERTY(int queryCount MEMBER queryCount_)
	Q_PROPERTY(ulong startTimeout MEMBER startTimeout_)
	Q_PROPERTY(ulong stopTimeout MEMBER stopTimeout_)
	Q_PROPERTY(ulong ttlChangeTimeout MEMBER ttlChangeTimeout_)
	Q_PROPERTY(ulong sendTimeout MEMBER sendTimeout_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)
	Q_PROPERTY(GObjPtr ipMgr READ getIpMgr WRITE setIpMgr)
	Q_PROPERTY(GObjPtr rawIpSockerWrite READ getRawIpSockerWrite WRITE setRawIpSockerWrite )

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }
	GObjPtr getIpMgr() { return ipMgr_; }
	void setIpMgr(GObjPtr value) { ipMgr_ = dynamic_cast<GIpMgr*>(value.data()); }
	GObjPtr getRawIpSockerWrite() { return rawIpSocketWrite_; }
	void setRawIpSockerWrite(GObjPtr value) { rawIpSocketWrite_ = dynamic_cast<GRawIpSocketWrite*>(value.data()); }

public:
	int maxHop_{30};
	int queryCount_{3};
	GDuration startTimeout_{5000};
	GDuration stopTimeout_{5000};
	GDuration ttlChangeTimeout_{1};
	GDuration sendTimeout_{1};
	GPcapDevice* pcapDevice_{nullptr};
	GIpMgr* ipMgr_{nullptr};
	GRawIpSocketWrite* rawIpSocketWrite_{nullptr};

public:
	Q_INVOKABLE GTraceRoute(QObject* parent = nullptr);
	~GTraceRoute() override;

protected:
	bool doOpen() override;
	bool doClose() override;

	GIntf* intf_{nullptr};
	GIp myIp_{0};

public:
	// Managable
	size_t ipOffset_{0};
	void ipCreated(GIp ip, GIpMgr::IpValue* ipValue) override;
	void ipDeleted(GIp ip, GIpMgr::IpValue* ipValue) override;

	// ------------------------------------------------------------------------
	// Item
	// ------------------------------------------------------------------------
	struct Item {
		bool active_{false};
	};
	typedef Item *PItem;
	// ------------------------------------------------------------------------

protected:
	void processPacketResponse(GPacket* packet, bool* ok);
	void processCreateThread(GPacket* packet);

public slots:
	void test(GPacket* packet);

protected:
	// ------------------------------------------------------------------------
	// HopMap
	// ------------------------------------------------------------------------
	struct HopMap : QMap<int /*hopNo*/, GIp> {
	};

	// ------------------------------------------------------------------------
	// TestThread
	// ------------------------------------------------------------------------
	struct TestThread : GThread {
		TestThread(GTraceRoute* tr, GPacket* packet, GFlow::IpFlowKey testFlowKey);
		~TestThread() override;
		void run() override;

		GFlow::IpFlowKey testFlowKey_;
		GAnyPacket anyPacket_;
		GPacket* sendPacket_;
		QByteArray sendPacketByteArray_;
		GStateWaitEvent swe_;
	};
	struct TestThreadMap : QMap<GFlow::IpFlowKey, TestThread*>, QRecursiveMutex {
	} testThreadMap_;
};
typedef GTraceRoute *PTraceRoute;
