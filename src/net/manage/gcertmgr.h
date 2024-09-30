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

#include "base/gstateobj.h"
#include "net/manage/gtcpflowmgr.h"
#include "net/pdu/gtcpsegment.h"
#include "net/pdu/gtls.h"

// ----------------------------------------------------------------------------
// GCertMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GCertMgr : GStateObj, GTcpFlowMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(QString caDirectory MEMBER caDirectory_)
	Q_PROPERTY(QString intermediateDirectory MEMBER intermediateDirectory_)
	Q_PROPERTY(QString serverDirectory MEMBER serverDirectory_)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)

	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }

public:
	QString caDirectory_;
	QString intermediateDirectory_;
	QString serverDirectory_;
	GTcpFlowMgr* tcpFlowMgr_{nullptr};

public:
	Q_INVOKABLE GCertMgr(QObject* parent = nullptr) : GStateObj(parent) {}
	~GCertMgr() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item : GTcpSegment {
		bool handshakeFinished_{false};
		Item(uint32_t seq) : GTcpSegment(seq) {}
		~Item() {}
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

	// GTcpFlowMgr::Managable
	size_t tcpFlowOffset_{0};
	Item* getItem(GTcpFlowMgr::TcpFlowValue* tcpFlowValue) { return PItem(tcpFlowValue->mem(tcpFlowOffset_)); }
	void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;

public slots:
	void manage(GPacket* packet);

signals:
	void managed(GTls::Handshake* hs);
};
