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
struct G_EXPORT GTcpFlowMgr : GPacketMgr {
	Q_OBJECT
	Q_PROPERTY(long halfTimeout MEMBER halfTimeout_)
	Q_PROPERTY(long fullTimeout MEMBER fullTimeout_)
	Q_PROPERTY(long rstTimeout MEMBER rstTimeout_)
	Q_PROPERTY(long finTimeout MEMBER finTimeout_)

public:
	long halfTimeout_{60 * 1}; // 1 minutes
	long fullTimeout_{60 * 60}; // 1 hour
	long rstTimeout_{10}; // 10 seconds
	long finTimeout_{20}; // 20 seconds

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GPacketMgr::Value* value) = 0;
		virtual void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GPacketMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::TcpFlowKey, GPacketMgr::Value*> {
		void clear() {
			for (GPacketMgr::Value* value: *this) {
				GPacketMgr::Value::deallocate(value);
			}
			QMap<GFlow::TcpFlowKey, GPacketMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GPacketMgr::Value* value = it.value();
			GPacketMgr::Value::deallocate(value);
			return QMap<GFlow::TcpFlowKey, GPacketMgr::Value*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GTcpFlowMgr(QObject* parent = nullptr) : GPacketMgr(parent) {}
	~GTcpFlowMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacketMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GFlow::TcpFlowKey tcpFlowKey_;
	GPacketMgr::Value* val_{nullptr};
	GFlow::TcpFlowKey rTcpFlowKey_;
	GPacketMgr::Value* rVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
