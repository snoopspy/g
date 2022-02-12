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
struct G_EXPORT GUdpFlowMgr : GPktMgr {
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
		virtual void udpFlowCreated(GFlow::UdpFlowKey* key, GPktMgr::Value* value) = 0;
		virtual void udpFlowDeleted(GFlow::UdpFlowKey* key, GPktMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::UdpFlowKey, GPktMgr::Value*> {
		void clear() {
			for (GPktMgr::Value* value: *this) {
				GPktMgr::Value::deallocate(value);
			}
			QMap<GFlow::UdpFlowKey, GPktMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GPktMgr::Value* value = it.value();
			GPktMgr::Value::deallocate(value);
			return QMap<GFlow::UdpFlowKey, GPktMgr::Value*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GUdpFlowMgr(QObject* parent = nullptr) : GPktMgr(parent) {}
	~GUdpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPktMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GFlow::UdpFlowKey key_;
	GPktMgr::Value* val_{nullptr};
	GFlow::UdpFlowKey rKey_;
	GPktMgr::Value* rVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
