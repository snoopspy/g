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
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void ipFlowCreated(GFlow::IpFlowKey* key, GPacketMgr::Value* value) = 0;
		virtual void ipFlowDeleted(GFlow::IpFlowKey* key, GPacketMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::IpFlowKey, GPacketMgr::Value*> {
		void clear() {
			for (GPacketMgr::Value* value: *this) {
				GPacketMgr::Value::deallocate(value);
			}
			QMap<GFlow::IpFlowKey, GPacketMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GPacketMgr::Value* value = it.value();
			GPacketMgr::Value::deallocate(value);
			return QMap<GFlow::IpFlowKey, GPacketMgr::Value*>::erase(it);
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
	GFlow::IpFlowKey key_;
	GPacketMgr::Value* val_{nullptr};
	GFlow::IpFlowKey rKey_;
	GPacketMgr::Value* rVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
