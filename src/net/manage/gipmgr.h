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
// GIpMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GIpMgr : GPacketMgr {
	Q_OBJECT
	Q_PROPERTY(long timeoutSec MEMBER timeoutSec_)
	Q_PROPERTY(Category category MEMBER category_)
	Q_PROPERTY(GObjPtr pcapDevice READ getPcapDevice WRITE setPcapDevice)
	Q_ENUMS(Category)

	GObjPtr getPcapDevice() { return pcapDevice_; }
	void setPcapDevice(GObjPtr value) { pcapDevice_ = dynamic_cast<GPcapDevice*>(value.data()); }

public:
	enum Category {
		Lan,
		Wan,
		All
	};

	bool enabled_{true};
	long timeoutSec_{70}; // 1 minute + 10 seconds
	Category category_{All};
	GPcapDevice* pcapDevice_{nullptr};

public:
	// --------------------------------------------------------------------------
	// IpValue
	// --------------------------------------------------------------------------
	struct IpValue : GPacketMgr::Value {
		// int foo; // gilgil temp 2023.12.04
#ifdef _DEBUG
		size_t totalMemSize_{0};
#endif // _DEBUG

		static struct IpValue* allocate(size_t totalMemSize) {
			IpValue* ipValue = reinterpret_cast<IpValue*>(malloc(sizeof(IpValue) + totalMemSize));
#ifdef _DEBUG
			ipValue->totalMemSize_ = totalMemSize;
			memset(pbyte(ipValue) + sizeof(IpValue), 'A', totalMemSize);
#endif // _DEBUG
			new (ipValue) IpValue;
			return ipValue;
		}

		static void deallocate(IpValue* ipValue) {
#ifdef _DEBUG
			memset(pbyte(ipValue) + sizeof(IpValue), 'B', ipValue->totalMemSize_);
#endif // _DEBUG
			ipValue->~IpValue();
			free(static_cast<void*>(ipValue));
		}

		void* mem(size_t offset) { return pbyte(this) + sizeof(IpValue) + offset; }
	};

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void ipCreated(GIp ip, GIpMgr::IpValue* ipValue) = 0;
		virtual void ipDeleted(GIp ip, GIpMgr::IpValue* ipValue) = 0;
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
	// IpMap
	// --------------------------------------------------------------------------
	struct IpMap : QMap<GIp, IpValue*>, QRecursiveMutex {
		void clear() {
			for (IpValue* ipValue: *this) {
				IpValue::deallocate(ipValue);
			}
			QMap<GIp, IpValue*>::clear();
		}

		IpMap::iterator erase(IpMap::iterator it) {
			IpValue* ipValue = it.value();
			IpValue::deallocate(ipValue);
			return QMap<GIp, IpValue*>::erase(it);
		}
	} ipMap_;
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GIpMgr(QObject* parent = nullptr) : GPacketMgr(parent) {}
	~GIpMgr() override { close(); }

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
	void processIp(GPacket* packet, GIp ip);

public:
	GIp currentIp_;
	IpValue* currentIpVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);

public:
	bool propLoad(QJsonObject jo, QMetaProperty mpro) override;
};
