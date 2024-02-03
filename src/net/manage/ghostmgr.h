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

#include "gpacketmgr.h"
#include "net/capture/gpcapdevice.h"

// ----------------------------------------------------------------------------
// GHostMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GHostMgr : GPacketMgr {
	Q_OBJECT
	Q_PROPERTY(ulong timeoutSec MEMBER timeoutSec_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }

public:
	bool enabled_{true};
	GDuration timeoutSec_{70}; // 1 minute + 10 seconds
	GPcapDevice* pcapDevice_{nullptr};

public:
	// --------------------------------------------------------------------------
	// HostValue
	// --------------------------------------------------------------------------
	struct HostValue : GPacketMgr::Value {
		GIp ip_{0};
		QString host_;
		QString vendor_;
#ifdef _DEBUG
		size_t totalMemSize_{0};
#endif // _DEBUG

		static struct HostValue* allocate(size_t totalMemSize) {
			HostValue* hostValue = PHostValue(malloc(sizeof(HostValue) + totalMemSize));
#ifdef _DEBUG
			hostValue->totalMemSize_ = totalMemSize;
			memset(pbyte(hostValue) + sizeof(HostValue), 'A', totalMemSize);
#endif // _DEBUG
			new (hostValue) HostValue;
			return hostValue;
		}

		static void deallocate(HostValue* hostValue) {
#ifdef _DEBUG
			memset(pbyte(hostValue) + sizeof(HostValue), 'B', hostValue->totalMemSize_);
#endif // _DEBUG
			hostValue->~HostValue();
			free(static_cast<void*>(hostValue));
		}

		void* mem(size_t offset) { return pbyte(this) + sizeof(HostValue) + offset; }
	};
	typedef HostValue *PHostValue;

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) = 0;
		virtual void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) = 0;
		virtual void hostChanged(GMac mac, GHostMgr::HostValue* hostValue) = 0;
	};
	struct Managables : QList<Managable*> {
		void insert(Managable* managable) {
			for (Managable* m: *this) {
				if (m == managable)
					return;
			}
			push_back(managable);
		}
	} managables_;
	// --------------------------------------------------------------------------

public:
	// --------------------------------------------------------------------------
	// HostMap
	// --------------------------------------------------------------------------
	struct HostMap : QMap<GMac, HostValue*>, QRecursiveMutex {
		void clear() {
			for (HostValue* hostValue: *this) {
				HostValue::deallocate(hostValue);
			}
			QMap<GMac, HostValue*>::clear();
		}

		HostMap::iterator erase(HostMap::iterator it) {
			HostValue* hostValue = it.value();
			HostValue::deallocate(hostValue);
			return QMap<GMac, HostValue*>::erase(it);
		}
	} hostMap_;
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GHostMgr(QObject* parent = nullptr) : GPacketMgr(parent) {}
	~GHostMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacketMgr::RequestItems requestItems_;

protected:
	GIntf* intf_{nullptr};
	GIp myIp_{0};
	GMac myMac_{GMac::nullMac()};
	GIp gwIp_{0};

	void deleteOldHosts(time_t now);

protected:
	bool processDhcp(GPacket* packet, GMac* mac, GIp* ip, QString* host, QString* alias);
	bool processArp(GEthHdr* ethHdr, GArpHdr* arpHdr, GMac* mac, GIp* ip);
	bool processIp(GEthHdr* ethHdr, GIpHdr* ipHdr, GMac* mac, GIp* ip);

public:
	GMac currentMac_;
	HostValue* currentHostVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);

public:
	bool propLoad(QJsonObject jo, QMetaProperty mpro) override;
};
