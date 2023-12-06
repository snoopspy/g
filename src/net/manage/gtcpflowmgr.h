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
	// TcpFlowValue
	// --------------------------------------------------------------------------
	struct TcpFlowValue : GPacketMgr::Value {
		enum State {
			Half,
			Full,
			Rst,
			Fin
		} state_;

		static struct TcpFlowValue* allocate(size_t totalMemSize) {
			TcpFlowValue* res = reinterpret_cast<TcpFlowValue*>(malloc(sizeof(TcpFlowValue) + totalMemSize));
			new (res) TcpFlowValue;
			return res;
		}

		static void deallocate(TcpFlowValue* tcpFlowValue) {
			tcpFlowValue->~TcpFlowValue();
			free(static_cast<void*>(tcpFlowValue));
		}

		void* mem(size_t offset) { return pbyte(this) + sizeof(TcpFlowValue) + offset; }
	};

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) = 0;
		virtual void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) = 0;
	};
	typedef QSet<Managable*> Managables;
	Managables managables_;
	// --------------------------------------------------------------------------

protected:
	// --------------------------------------------------------------------------
	// FlowMap
	// --------------------------------------------------------------------------
	struct FlowMap : QMap<GFlow::TcpFlowKey, TcpFlowValue*> {
		void clear() {
			for (TcpFlowValue* tcpFlowValue: *this) {
				TcpFlowValue::deallocate(tcpFlowValue);
			}
			QMap<GFlow::TcpFlowKey, TcpFlowValue*>::clear();
		}

		FlowMap::iterator erase(FlowMap::iterator it) {
			TcpFlowValue* tcpFlowValue = it.value();
			TcpFlowValue::deallocate(tcpFlowValue);
			return QMap<GFlow::TcpFlowKey, TcpFlowValue*>::erase(it);
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
	void deleteOldFlowMaps(time_t now);

public:
	GFlow::TcpFlowKey currentTcpFlowKey_;
	TcpFlowValue* currentTcpFlowVal_{nullptr};
	GFlow::TcpFlowKey currentRevTcpFlowKey_;
	TcpFlowValue* currentRevTcpFlowVal_{nullptr};

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GPacket* packet);
};
