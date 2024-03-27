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

#include <QRegularExpression>

#include "base/gstateobj.h"
#include "net/pdu/gdnsinfo.h"
#include "net/write/grawipsocketwrite.h"

// ----------------------------------------------------------------------------
// GDnsBlockItem
// ----------------------------------------------------------------------------
struct GDnsBlockItem : GObj {
	Q_OBJECT
	Q_PROPERTY(QString domain MEMBER domain_)
	Q_PROPERTY(QString ip MEMBER ip_)

public:
	QString domain_;
	QString ip_{"127.0.0.1"};

public:
	Q_INVOKABLE GDnsBlockItem(QObject* parent = nullptr);
	Q_INVOKABLE GDnsBlockItem(QObject* parent, QString domain, QString ip);
	~GDnsBlockItem() override;
};
typedef GDnsBlockItem *PDnsBlockItem;

// ----------------------------------------------------------------------------
// GDnsBlock
// ----------------------------------------------------------------------------
struct G_EXPORT GDnsBlock : GStateObj {
	Q_OBJECT
	Q_PROPERTY(quint16 port MEMBER port_)
	Q_PROPERTY(GObjRefArrayPtr items READ getItems)
	Q_PROPERTY(GObjRef writer READ getWriter)

	GObjRefArrayPtr getItems() { return &dnsBlockItems_; }
	GObjRef getWriter() { return &writer_; }

public:
	uint16_t port_{53};
	GObjRefArray<GDnsBlockItem> dnsBlockItems_;
	GRawIpSocketWrite writer_{this};

public:
	Q_INVOKABLE GDnsBlock(QObject* parent = nullptr) : GStateObj(parent) {}
	~GDnsBlock() override { close(); }

protected:
	struct MyBlockItem {
		QRegularExpression re_;
		GIp ip_;
	};
	typedef MyBlockItem *PMyBlockItem;

	struct MyBlockItems : QList<MyBlockItem> {
	} myBlockItems_;

	GIpPacket blockIpPacket_;
	QByteArray blockByteArray_;

protected:
	bool doOpen() override;
	bool doClose() override;

public slots:
	void block(GPacket* packet);

signals:
	void blocked(GPacket* packet);
};
