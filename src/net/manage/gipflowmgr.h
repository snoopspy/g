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
// GIpFlowMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GIpFlowMgr : GPacketMgr {
	Q_OBJECT
	Q_PROPERTY(long halfTimeout MEMBER halfTimeout_)
	Q_PROPERTY(long fullTimeout MEMBER fullTimeout_)

public:
	long halfTimeout_{60 * 1}; // 1 minutes
	long fullTimeout_{60 * 3}; // 3 minutes

public:
	// --------------------------------------------------------------------------
	// IpFlowValue
	// --------------------------------------------------------------------------
	struct IpFlowValue : GPacketMgr::Value {
		enum State {
			Half,
			Full
		} state_;

		static struct IpFlowValue* allocate(size_t totalMemSize) {
			IpFlowValue* res = reinterpret_cast<IpFlowValue*>(malloc(sizeof(IpFlowValue) + totalMemSize));
			new (res) IpFlowValue;
			return res;
		}

		static void deallocate(IpFlowValue* ipFlowValue) {
			ipFlowValue->~IpFlowValue();
			free(static_cast<void*>(ipFlowValue));
		}

		void* mem(size_t offset) { return pbyte(this) + sizeof(IpFlowValue) + offset; }
	};

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void ipFlowCreated(GFlow::IpFlowKey ipFlowKey, GIpFlowMgr::IpFlowValue* ipFlowValue) = 0;
		virtual void ipFlowDeleted(GFlow::IpFlowKey ipFlowKey, GIpFlowMgr::IpFlowValue* ipFlowValue) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::IpFlowKey, IpFlowValue*> {
		void clear() {
			for (IpFlowValue* ipFlowValue: *this) {
				IpFlowValue::deallocate(ipFlowValue);
			}
			QMap<GFlow::IpFlowKey, IpFlowValue*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			IpFlowValue* ipFlowValue = it.value();
			IpFlowValue::deallocate(ipFlowValue);
			return QMap<GFlow::IpFlowKey, IpFlowValue*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GIpFlowMgr(QObject* parent = nullptr) : GPacketMgr(parent) {}
	~GIpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacketMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GFlow::IpFlowKey currentIpFlowKey_;
	IpFlowValue* currentIpFlowVal_{nullptr};
	GFlow::IpFlowKey currentRevIpFlowKey_;
	IpFlowValue* currentRevIpFlowVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
