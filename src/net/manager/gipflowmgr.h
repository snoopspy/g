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
// GIpFlowMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GIpFlowMgr : GMgr {
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
		virtual void ipFlowCreated(GFlow::IpFlowKey* key, GMgr::Value* value) = 0;
		virtual void ipFlowDeleted(GFlow::IpFlowKey* key, GMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::IpFlowKey, GMgr::Value*> {
		void clear() {
			for (GMgr::Value* value: *this) {
				GMgr::Value::deallocate(value);
			}
			QMap<GFlow::IpFlowKey, GMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GMgr::Value* value = it.value();
			GMgr::Value::deallocate(value);
			return QMap<GFlow::IpFlowKey, GMgr::Value*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GIpFlowMgr(QObject* parent = nullptr) : GMgr(parent) {}
	~GIpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GFlow::IpFlowKey key_;
	GMgr::Value* val_{nullptr};
	GFlow::IpFlowKey rKey_;
	GMgr::Value* rVal_{nullptr};

public slots:
	void process(GPacket* packet);

signals:
	void processed(GPacket* packet);
};
