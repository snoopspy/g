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

// ----------------------------------------------------------------------------
// GUdpFlowMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GUdpFlowMgr : GPacketMgr {
	Q_OBJECT
	Q_PROPERTY(long halfTimeout MEMBER halfTimeout_)
	Q_PROPERTY(long fullTimeout MEMBER fullTimeout_)

public:
	long halfTimeout_{60 * 1}; // 1 minutes
	long fullTimeout_{60 * 3}; // 3 minutes

public:
	// --------------------------------------------------------------------------
	// UdpFlowValue
	// --------------------------------------------------------------------------
	struct UdpFlowValue : GPacketMgr::Value {
		enum State {
			Half,
			Full
		} state_;

		static struct UdpFlowValue* allocate(size_t totalMemSize) {
			UdpFlowValue* udpFlowValue = PUdpFlowValue(malloc(sizeof(UdpFlowValue) + totalMemSize));
			new (udpFlowValue) UdpFlowValue;
#ifdef _DEBUG
			udpFlowValue->totalMemSize_ = totalMemSize;
			memset(pbyte(udpFlowValue) + sizeof(UdpFlowValue), 'A', totalMemSize);
#endif // _DEBUG
			return udpFlowValue;
		}

		static void deallocate(UdpFlowValue* udpFlowValue) {
#ifdef _DEBUG
			memset(pbyte(udpFlowValue) + sizeof(UdpFlowValue), 'B', udpFlowValue->totalMemSize_);
#endif // _DEBUG
			udpFlowValue->~UdpFlowValue();
			free(static_cast<void*>(udpFlowValue));
		}

		void* mem(size_t offset) { return pbyte(this) + sizeof(UdpFlowValue) + offset; }
	};
	typedef UdpFlowValue *PUdpFlowValue;

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void udpFlowCreated(GFlow::UdpFlowKey udpFlowKey, GUdpFlowMgr::UdpFlowValue* udpFlowValue) = 0;
		virtual void udpFlowDeleted(GFlow::UdpFlowKey udpFlowKey, GUdpFlowMgr::UdpFlowValue* udpFlowValue) = 0;
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

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::UdpFlowKey, UdpFlowValue*> {
		void clear() {
			for (UdpFlowValue* udpFlowValue: *this) {
				UdpFlowValue::deallocate(udpFlowValue);
			}
			QMap<GFlow::UdpFlowKey, UdpFlowValue*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			UdpFlowValue* udpFlowValue = it.value();
			UdpFlowValue::deallocate(udpFlowValue);
			return QMap<GFlow::UdpFlowKey, UdpFlowValue*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GUdpFlowMgr(QObject* parent = nullptr) : GPacketMgr(parent) {}
	~GUdpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacketMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(time_t now);

public:
	GFlow::UdpFlowKey currentUdpFlowkey_;
	UdpFlowValue* currentUdpFlowVal_{nullptr};
	GFlow::UdpFlowKey currentRevUdpFlowKey_;
	UdpFlowValue* currentRevUdpFlowVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
