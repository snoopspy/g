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
// GTcpFlowMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GTcpFlowMgr : GPktMgr {
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
		virtual void tcpFlowCreated(GFlow::TcpFlowKey* key, GPktMgr::Value* value) = 0;
		virtual void tcpFlowDeleted(GFlow::TcpFlowKey* key, GPktMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::TcpFlowKey, GPktMgr::Value*> {
		void clear() {
			for (GPktMgr::Value* value: *this) {
				GPktMgr::Value::deallocate(value);
			}
			QMap<GFlow::TcpFlowKey, GPktMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GPktMgr::Value* value = it.value();
			GPktMgr::Value::deallocate(value);
			return QMap<GFlow::TcpFlowKey, GPktMgr::Value*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GTcpFlowMgr(QObject* parent = nullptr) : GPktMgr(parent) {}
	~GTcpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPktMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GFlow::TcpFlowKey key_;
	GPktMgr::Value* val_{nullptr};
	GFlow::TcpFlowKey rKey_;
	GPktMgr::Value* rVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
