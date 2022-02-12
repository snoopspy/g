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

#include "gpktmgr.h"

// ----------------------------------------------------------------------------
// GIpFlowMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GIpFlowMgr : GPktMgr {
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
		virtual void ipFlowCreated(GFlow::IpFlowKey* key, GPktMgr::Value* value) = 0;
		virtual void ipFlowDeleted(GFlow::IpFlowKey* key, GPktMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::IpFlowKey, GPktMgr::Value*> {
		void clear() {
			for (GPktMgr::Value* value: *this) {
				GPktMgr::Value::deallocate(value);
			}
			QMap<GFlow::IpFlowKey, GPktMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GPktMgr::Value* value = it.value();
			GPktMgr::Value::deallocate(value);
			return QMap<GFlow::IpFlowKey, GPktMgr::Value*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GIpFlowMgr(QObject* parent = nullptr) : GPktMgr(parent) {}
	~GIpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPktMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GFlow::IpFlowKey key_;
	GPktMgr::Value* val_{nullptr};
	GFlow::IpFlowKey rKey_;
	GPktMgr::Value* rVal_{nullptr};

public slots:
	void process(GPacket* packet);

signals:
	void processed(GPacket* packet);
};
