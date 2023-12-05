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

#include "base/sys/gthread.h"
#include "base/sys/gwaitevent.h"
#include "net/gnetinfo.h"
#include "net/manage/gflowkey.h"
#include "net/packet/gpacket.h"
#include "net/write/grawipsocketwrite.h"

// ----------------------------------------------------------------------------
// GTraceRoute
// ----------------------------------------------------------------------------
struct G_EXPORT GTraceRoute : GStateObj {
	Q_OBJECT
	Q_PROPERTY(QString intfName MEMBER intfName_)
	Q_PROPERTY(int maxHop MEMBER maxHop_)
	Q_PROPERTY(int queryCount MEMBER queryCount_)
	Q_PROPERTY(ulong startTimeout MEMBER startTimeout_)
	Q_PROPERTY(ulong stopTimeout MEMBER stopTimeout_)
	Q_PROPERTY(ulong ttlChangeTimeout MEMBER ttlChangeTimeout_)
	Q_PROPERTY(ulong sendTimeout MEMBER sendTimeout_)
	Q_PROPERTY(quint16 tcpLocalPort MEMBER tcpLocalPort_)
	Q_PROPERTY(quint16 udpLocalPort MEMBER udpLocalPort_)
	Q_PROPERTY(quint16 icmpId MEMBER icmpId_)
	Q_PROPERTY(QString logFileName MEMBER logFileName_)
	Q_PROPERTY(GObjPtr rawIpSockerWrite READ getRawIpSockerWrite WRITE setRawIpSockerWrite )

	GObjPtr getRawIpSockerWrite() { return rawIpSocketWrite_; }
	void setRawIpSockerWrite(GObjPtr value) { rawIpSocketWrite_ = dynamic_cast<GRawIpSocketWrite*>(value.data()); }

public:
	QString intfName_{""};
	int maxHop_{30};
	int queryCount_{3};
	GDuration startTimeout_{5000};
	GDuration stopTimeout_{5000};
	GDuration ttlChangeTimeout_{1};
	GDuration sendTimeout_{1};
	uint16_t tcpLocalPort_{0};
	uint16_t udpLocalPort_{0};
	uint16_t icmpId_{0};
	QString logFileName_{"tr.tsv"};
	GRawIpSocketWrite* rawIpSocketWrite_{nullptr};

public:
	Q_INVOKABLE GTraceRoute(QObject* parent = nullptr);
	~GTraceRoute() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GIntf* intf_{nullptr};
	GIp myIp_{0};

protected:
	void processHostResponse(GPacket* packet, bool* ok);
	void processTtlResponse(GPacket* packet, bool* ok);
	void processCreateThread(GPacket* packet);

public slots:
	void probe(GPacket* packet);

protected:
	// ------------------------------------------------------------------------
	// Key
	// ------------------------------------------------------------------------
	struct TransportKey {
		GIp sip_;
		GIp dip_;
		uint16_t port_;

		TransportKey() {}
		TransportKey(GIp sip, GIp dip, uint16_t port) : sip_(sip), dip_(dip), port_(port) {}

		bool operator < (const TransportKey& r) const {
			if (this->sip_ < r.sip_) return true;
			if (this->sip_ > r.sip_) return false;
			if (this->dip_ < r.dip_) return true;
			if (this->dip_ > r.dip_) return false;
			if (this->port_ < r.port_) return true;
			return false;
		};
	};
	typedef TransportKey TcpKey;
	typedef TransportKey UdpKey;
	typedef GFlow::IpFlowKey IcmpKey;

	// ------------------------------------------------------------------------
	// HopMap
	// ------------------------------------------------------------------------
	struct HopMap : QMap<int /*hopNo*/, GIp> {
		int maxHopNo() {
			int res = 0;
			for(HopMap::iterator it = begin(); it != end(); it++) {
				int hopNo = it.key();
				if (res < hopNo)
					res = hopNo;
			}
			qDebug() << res;
			return res;
		}
	};

	// ------------------------------------------------------------------------
	// ProbeThread
	// ------------------------------------------------------------------------
	struct ProbeThread : GThread {
		ProbeThread(GTraceRoute* tr);
		~ProbeThread() override;

		GIpPacket sendPacket_;
		QByteArray sendPacketByteArray_;
		GStateWaitEvent swe_;

		HopMap hopMap_;
		GIp hostHopIp_;
		int hostHopNo_{0};

		virtual QString logHeader() = 0;
		void writeLog();
	};

	// ------------------------------------------------------------------------
	// TcpThread
	// ------------------------------------------------------------------------
	struct TcpThread : ProbeThread {
		TcpKey tcpKey_;
		uint16_t sport_;

		TcpThread(GTraceRoute* tr, TcpKey tcpKey, GPacket* packet);
		~TcpThread() override;
		void run() override;
		bool processHostResponse(GTcpHdr* tcpHdr);
		bool processTtlResponse(GIpHdr* ipHdr, GIpHdr* ipHdr2, GTcpHdr* tcpHdr2);

		QString logHeader() override {
			return QString("TCP\t%1:%2\t%3:%4\t").arg(QString(tcpKey_.sip_)).arg(sport_).arg(QString(tcpKey_.dip_)).arg(tcpKey_.port_);
		}
	};

	// ------------------------------------------------------------------------
	// UdpThread
	// ------------------------------------------------------------------------
	struct UdpThread : ProbeThread {
		UdpKey udpKey_;
		uint16_t sport_;

		UdpThread(GTraceRoute* tr, UdpKey udpKey, GPacket* packet);
		~UdpThread() override;
		void run() override;

		QString logHeader() override {
			return QString("UDP\t%1:%2\t%3:%4\t").arg(QString(udpKey_.sip_)).arg(sport_).arg(QString(udpKey_.dip_)).arg(udpKey_.port_);
		}
	};

	// ------------------------------------------------------------------------
	// UdpThread
	// ------------------------------------------------------------------------
	struct IcmpThread : ProbeThread {
		IcmpKey icmpKey_;
		uint16_t id_;

		IcmpThread(GTraceRoute* tr, IcmpKey icmpKey, GPacket* packet);
		~IcmpThread() override;
		void run() override;
		bool processHostResponse(GIcmpPingHdr* icmpPingHdr);

		QString logHeader() override {
			return QString("ICMP\t%1:%2\t%3:%4\t").arg(QString(icmpKey_.sip_)).arg(id_).arg(QString(icmpKey_.dip_)).arg(id_);
		}
	};

protected:
	struct TcpMgr : QMap<TcpKey, TcpThread*>, QRecursiveMutex {} tcpMgr_;
	struct UdpMgr : QMap<UdpKey, UdpThread*>, QRecursiveMutex {} udpMgr_;
	struct IcmpMgr : QMap<IcmpKey, IcmpThread*>, QRecursiveMutex {} icmpMgr_;

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
typedef GTraceRoute *PTraceRoute;
