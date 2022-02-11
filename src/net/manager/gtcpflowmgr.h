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
// GTcpFlowMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GTcpFlowMgr : GMgr {
	Q_OBJECT
	Q_PROPERTY(long halfTimeout MEMBER halfTimeout_)
	Q_PROPERTY(long fullTimeout MEMBER fullTimeout_)
	Q_PROPERTY(long rstTimeout MEMBER rstTimeout_)
	Q_PROPERTY(long finTimeout MEMBER finTimeout_)

public:
	long halfTimeout_{60 * 1}; // 1 minutes
	long fullTimeout_{60 * 3}; // 3 minutes
	long rstTimeout_{10}; // 10 seconds
	long finTimeout_{20}; // 20 seconds

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void tcpFlowCreated(GFlow::TcpFlowKey* key, GMgr::Value* value) = 0;
		virtual void tcpFlowDeleted(GFlow::TcpFlowKey* key, GMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::TcpFlowKey, GMgr::Value*> {
		void clear() {
			for (GMgr::Value* value: *this) {
				GMgr::Value::deallocate(value);
			}
			QMap<GFlow::TcpFlowKey, GMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GMgr::Value* value = it.value();
			GMgr::Value::deallocate(value);
			return QMap<GFlow::TcpFlowKey, GMgr::Value*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GTcpFlowMgr(QObject* parent = nullptr) : GMgr(parent) {}
	~GTcpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GFlow::TcpFlowKey key_;
	GMgr::Value* val_{nullptr};
	GFlow::TcpFlowKey rKey_;
	GMgr::Value* rVal_{nullptr};

public slots:
	void process(GPacket* packet);

signals:
	void processed(GPacket* packet);
};
