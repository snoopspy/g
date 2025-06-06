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
	Q_PROPERTY(bool saveCertFile MEMBER saveCertFile_)
	Q_PROPERTY(QString saveCertFileFolder MEMBER saveCertFileFolder_)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)

	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }

public:
	bool saveCertFile_{false};
	QString saveCertFileFolder_{QString("certificate") + QDir::separator()};
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
	struct Item {
		bool checkNeeded_{true};
		GTcpSegment segment_;
		QString serverName_;
		Item(uint32_t seq) : segment_(seq) {}
		~Item() {}
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

	// GTcpFlowMgr::Managable
	size_t tcpFlowOffset_{0};
	Item* getItem(GTcpFlowMgr::TcpFlowValue* tcpFlowValue) { return PItem(tcpFlowValue->mem(tcpFlowOffset_)); }
	void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;

	static QString extractServerName(GTls::Handshake *hs);
	static QList<QByteArray> extractCertificates(GTls::Handshake *hs);

	static bool makeFolder(QString& folder);
	static void saveCertFiles(QString folder, QString serverName, struct timeval ts, QList<QByteArray>& certs);

public slots:
	void manage(GPacket* packet);

signals:
	void handshakeDetected(GTls::Handshake* hs);
	void certificatesDetected(QString serverName, GIp serverIp, struct timeval st, QList<QByteArray> certs);
};
