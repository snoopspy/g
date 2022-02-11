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

#include "gmgr.h"

// ----------------------------------------------------------------------------
// GUdpFlowMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GUdpFlowMgr : GMgr {
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
		virtual void udpFlowCreated(GFlow::UdpFlowKey* key, GMgr::Value* value) = 0;
		virtual void udpFlowDeleted(GFlow::UdpFlowKey* key, GMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::UdpFlowKey, GMgr::Value*> {
		void clear() {
			for (GMgr::Value* value: *this) {
				GMgr::Value::deallocate(value);
			}
			QMap<GFlow::UdpFlowKey, GMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GMgr::Value* value = it.value();
			GMgr::Value::deallocate(value);
			return QMap<GFlow::UdpFlowKey, GMgr::Value*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GUdpFlowMgr(QObject* parent = nullptr) : GMgr(parent) {}
	~GUdpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GFlow::UdpFlowKey key_;
	GMgr::Value* val_{nullptr};
	GFlow::UdpFlowKey rKey_;
	GMgr::Value* rVal_{nullptr};

public slots:
	void process(GPacket* packet);

signals:
	void processed(GPacket* packet);
};
