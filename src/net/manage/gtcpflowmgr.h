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
	Q_PROPERTY(long forwardFinTimeout MEMBER forwardFinTimeout_)
	Q_PROPERTY(long backwardFinTimeout MEMBER backwardFinTimeout_)
	Q_PROPERTY(long fullFinTimeout MEMBER fullFinTimeout_)
	Q_PROPERTY(long forwardRstTimeout MEMBER forwardRstTimeout_)
	Q_PROPERTY(long backwardRstTimeout MEMBER backwardRstTimeout_)
	Q_PROPERTY(long fullRstTimeout MEMBER fullRstTimeout_)

public:
	long halfTimeout_{60 * 1}; // 1 minutes
	long fullTimeout_{60 * 60}; // 1 hour

	long forwardFinTimeout_{30}; // 30 seconds
	long backwardFinTimeout_{60}; // 60 seconds
	long fullFinTimeout_{15}; // 15 seconds

	long forwardRstTimeout_{10}; // 10 seconds
	long backwardRstTimeout_{20}; // 20 seconds
	long fullRstTimeout_{5}; // 5 seconds

public:
	// --------------------------------------------------------------------------
	// TcpFlowValue
	// --------------------------------------------------------------------------
	struct TcpFlowValue : GPacketMgr::Value {
		enum State {
			Half,
			Full,
			ForwardFin,
			BackwardFin,
			FullFin,
			ForwardRst,
			BackwardRst,
			FullRst
		} state_;

		Direction direction_;
		quint64 packets_{0};
		quint64 bytes_{0};

		static struct TcpFlowValue* allocate(size_t totalMemSize) {
			TcpFlowValue* tcpFlowValue = PTcpFlowValue(malloc(sizeof(TcpFlowValue) + totalMemSize));
			new (tcpFlowValue) TcpFlowValue;
#ifdef _DEBUG
			tcpFlowValue->totalMemSize_ = totalMemSize;
			memset(pbyte(tcpFlowValue) + sizeof(TcpFlowValue), 'A', totalMemSize);
#endif // _DEBUG
			return tcpFlowValue;
		}

		static void deallocate(TcpFlowValue* tcpFlowValue) {
#ifdef _DEBUG
			memset(pbyte(tcpFlowValue) + sizeof(TcpFlowValue), 'B', tcpFlowValue->totalMemSize_);
#endif // _DEBUG
			tcpFlowValue->~TcpFlowValue();
			free(static_cast<void*>(tcpFlowValue));
		}

		void* mem(size_t offset) { return pbyte(this) + sizeof(TcpFlowValue) + offset; }
	};
	typedef TcpFlowValue *PTcpFlowValue;

public:
	// --------------------------------------------------------------------------
	// Managable
	// --------------------------------------------------------------------------
	struct Managable {
		virtual void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) = 0;
		virtual void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) = 0;
	};
	struct Managables : QList<Managable*> {
		void insert(Managable* managable) {
			for (Managable* m: *this) {
				if (m == managable)
					return;
			}
			push_back(managable);
		}
	} managables_;
	// --------------------------------------------------------------------------

public:
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

public:
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
