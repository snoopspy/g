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
	Q_PROPERTY(long timeoutSec MEMBER timeoutSec_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }

public:
	bool enabled_{true};
	long timeoutSec_{60}; // 1 minutes
	GPcapDevice* pcapDevice_{nullptr};

public:
	// --------------------------------------------------------------------------
	// Value
	// --------------------------------------------------------------------------
	struct Value {
		struct timeval ts_;
		GIp ip_;
		u_char totalMem_[0];

		static struct Value* allocate(size_t totalMemSize) {
			Value* res = reinterpret_cast<Value*>(malloc(sizeof(struct Value) + totalMemSize));
			return res;
		}

		static void deallocate(Value* value) {
			free(static_cast<void*>(value));
		}

		void* mem(size_t offset) { return totalMem_ + offset; }
	};

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void hostDetected(GMac mac, GHostMgr::Value* value) = 0;
		virtual void hostDeleted(GMac mac, GHostMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

public:
	// --------------------------------------------------------------------------
	// HostMap
	// --------------------------------------------------------------------------
	struct HostMap : QMap<GMac, GHostMgr::Value*> {
		void clear() {
			for (GHostMgr::Value* value: *this) {
				GHostMgr::Value::deallocate(value);
			}
			QMap<GMac, GHostMgr::Value*>::clear();
		}

		HostMap::iterator erase(HostMap::iterator it) {
			GHostMgr::Value* value = it.value();
			GHostMgr::Value::deallocate(value);
			return QMap<GMac, GHostMgr::Value*>::erase(it);
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

	void deleteOldFlowMaps(long now);

protected:
	bool processArp(GEthHdr* ethHdr, GArpHdr* arpHdr, GMac* mac, GIp* ip);
	bool processIp(GEthHdr* ethHdr, GIpHdr* ipHdr, GMac* mac, GIp* ip);

public:
	GMac currentMac_;
	Value* currentVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
