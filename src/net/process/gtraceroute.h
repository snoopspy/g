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
#include "net/filter/gbpfilter.h"
#include "net/gnetinfo.h"
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
	Q_PROPERTY(ulong nextProbeTimeoutSec MEMBER nextProbeTimeoutSec_)
	Q_PROPERTY(QString logFileName MEMBER logFileName_)
	Q_PROPERTY(GObjRef filter READ getBpFilter)
	Q_PROPERTY(GObjRef write READ getRawIpSockerWrite)

	GObjRef getBpFilter() { return &bpFilter_; }
	GObjRef getRawIpSockerWrite() { return &rawIpSocketWrite_; }

public:
	QString intfName_{""};
	int maxHop_{30};
	int queryCount_{3};
	GDuration startTimeout_{100};
	GDuration stopTimeout_{1000};
	GDuration ttlChangeTimeout_{1};
	GDuration sendTimeout_{1};
	GDuration nextProbeTimeoutSec_{3600}; // 1 hour
	Q_PROPERTY(ulong nextProbeTimeoutSec MEMBER nextProbeTimeoutSec_)
	QString logFileName_{"tr.tsv"};
	GBpFilter bpFilter_{this};
	GRawIpSocketWrite rawIpSocketWrite_{this};

public:
	Q_INVOKABLE GTraceRoute(QObject* parent = nullptr);
	~GTraceRoute() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GIntf* intf_{nullptr};
	GIp myIp_{0};
	QMutex logFileMutex_;

protected:
	void checkTtlResponse(GPacket* packet, bool* ok);
	void checkCreateThread(GPacket* packet);

public slots:
	void probe(GPacket* packet);

protected:
	// ------------------------------------------------------------------------
	// Key
	// ------------------------------------------------------------------------
	struct Key {
		uint8_t p_; // tcp, udp or icmp
		GIp dip_;

		Key() {}
		Key(uint8_t p , GIp dip) : p_(p), dip_(dip) {}

		bool operator < (const Key& r) const {
			if (this->dip_ < r.dip_) return true;
			if (this->dip_ > r.dip_) return false;
			if (this->p_ < r.p_) return true;
			return false;
		};
	};

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
			return res;
		}
	};

	// ------------------------------------------------------------------------
	// ProbeThread
	// ------------------------------------------------------------------------
	struct ProbeThread : GThread {
		ProbeThread(GTraceRoute* tr, GPacket* packet, Key key);
		~ProbeThread() override;

		void run() override;

		Key key_;
		GIpHdr* ipHdr_;

		GIpPacket sendPacket_;
		QByteArray sendPacketByteArray_;
		GStateWaitEvent swe_;
		HopMap hopMap_;

		bool checkTtlResponse(GIpHdr* ipHdr, GIpHdr* ipHdr2);

		QDateTime logTime_;
		virtual QString logHeader() = 0;
		void writeLog();
	};

	// ------------------------------------------------------------------------
	// TcpThread
	// ------------------------------------------------------------------------
	struct TcpThread : ProbeThread {
		TcpThread(GTraceRoute* tr, GPacket* packet, Key key);
		~TcpThread() override;

		GTcpHdr* tcpHdr_;
		GIp sip_;
		uint16_t sport_;
		GIp dip_;
		uint16_t dport_;

		QString logHeader() override {
			return QString("tcp\t%1\t%2\t%3\t%4\t").arg(QString(sip_)).arg(sport_).arg(QString(dip_)).arg(dport_);
		}
	};

	// ------------------------------------------------------------------------
	// UdpThread
	// ------------------------------------------------------------------------
	struct UdpThread : ProbeThread {
		UdpThread(GTraceRoute* tr, GPacket* packet, Key key);
		~UdpThread() override;

		GUdpHdr* udpHdr_;
		GIp sip_;
		uint16_t sport_;
		GIp dip_;
		uint16_t dport_;

		QString logHeader() override {
			return QString("udp\t%1\t%2\t%3\t%4\t").arg(QString(sip_)).arg(sport_).arg(QString(dip_)).arg(dport_);
		}
	};

	// ------------------------------------------------------------------------
	// UdpThread
	// ------------------------------------------------------------------------
	struct IcmpThread : ProbeThread {
		IcmpThread(GTraceRoute* tr, GPacket* packet, Key key);
		~IcmpThread() override;

		GIcmpPingHdr* icmpPingHdr_;
		GIp sip_;
		GIp dip_;
		uint16_t id_;

		QString logHeader() override {
			return QString("icmp\t%1\t%2\t%3\t%4\t").arg(QString(sip_)).arg(id_).arg(QString(dip_)).arg(id_);
		}
	};

protected:
	struct ThreadMgr : QMap<Key, ProbeThread*>, QRecursiveMutex {} threadMgr_;

	struct NextProbeMgr : QMap<Key, time_t>, QRecursiveMutex {
		GTraceRoute* tr_;
		NextProbeMgr(GTraceRoute* tr) : tr_(tr) {}
		void deleteOldProbes(time_t now) {
			NextProbeMgr::iterator it = begin();
			while (it != end()) {
				time_t t = it.value();
				ulong elapsed = now - t;
				if (elapsed >= tr_->nextProbeTimeoutSec_) {
					it = erase(it);
					continue;
				}
				it++;
			}
		}
	} nextProbeMgr_{this};

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
typedef GTraceRoute *PTraceRoute;
