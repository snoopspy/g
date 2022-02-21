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
// GHostMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GHostMgr : GPacketMgr {
	Q_OBJECT
	Q_PROPERTY(long timeout MEMBER timeout_)

public:
	long timeout_{60}; // 1 minutes

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void hostDetected(GMac mac, GPacketMgr::Value* value) = 0;
		virtual void hostDeleted(GMac mac, GPacketMgr::Value* value) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GMac, GPacketMgr::Value*> {
		void clear() {
			for (GPacketMgr::Value* value: *this) {
				GPacketMgr::Value::deallocate(value);
			}
			QMap<GMac, GPacketMgr::Value*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			GPacketMgr::Value* value = it.value();
			GPacketMgr::Value::deallocate(value);
			return QMap<GMac, GPacketMgr::Value*>::erase(it);
		}
	};
	// --------------------------------------------------------------------------

public:
	Q_INVOKABLE GHostMgr(QObject* parent = nullptr) : GPacketMgr(parent) {}
	~GHostMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacketMgr::RequestItems requestItems_;

protected:
	FlowMap flowMap_;
	void deleteOldFlowMaps(long now);

public:
	GMac mac_;
	GPacketMgr::Value* val_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
