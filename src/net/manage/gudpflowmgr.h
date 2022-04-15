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
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void udpFlowDetected(GFlow::UdpFlowKey udpFlowKey, GPacketMgr::Value* value) = 0;
		virtual void udpFlowDeleted(GFlow::UdpFlowKey udpFlowKey, GPacketMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::UdpFlowKey, GPacketMgr::Value*> {
		void clear() {
			for (GPacketMgr::Value* value: *this) {
				GPacketMgr::Value::deallocate(value);
			}
			QMap<GFlow::UdpFlowKey, GPacketMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GPacketMgr::Value* value = it.value();
			GPacketMgr::Value::deallocate(value);
			return QMap<GFlow::UdpFlowKey, GPacketMgr::Value*>::erase(it);
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
	void deleteOldFlowMaps(long now);

public:
	GFlow::UdpFlowKey currentUdpFlowkey_;
	GPacketMgr::Value* currentVal_{nullptr};
	GFlow::UdpFlowKey currentRevUdpFlowKey_;
	GPacketMgr::Value* currentRevVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
