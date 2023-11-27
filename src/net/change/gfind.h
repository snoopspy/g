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
#include "net/packet/gpacket.h"

// ----------------------------------------------------------------------------
// GFindItem
// ----------------------------------------------------------------------------
struct GFindItem : GObj {
	Q_OBJECT
	Q_PROPERTY(int offset MEMBER offset_)
	Q_PROPERTY(int endOffset MEMBER endOffset_)
	Q_PROPERTY(int count MEMBER count_)
	Q_PROPERTY(QString pattern MEMBER pattern_)
	Q_PROPERTY(Type type MEMBER type_)
	Q_ENUMS(Type)

public:
	enum Type {
		String,
		HexValue,
		RegularExpression
	};

public:
	int offset_{0};
	int endOffset_{-1};
	int count_{1};
	QString pattern_;
	Type type_{String};

public:
	QString findHexPattern_;
	QRegularExpression findRe_;
};
typedef GFindItem *PFindItem;

// ----------------------------------------------------------------------------
// GFind
// ----------------------------------------------------------------------------
struct G_EXPORT GFind : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool log MEMBER log_)
	Q_PROPERTY(GObjRefArrayPtr items READ getItems)

	virtual GObjRefArrayPtr getItems() { return &findItems_; }

public:
	bool log_{true};
	GObjRefArray<GFindItem> findItems_;

public:
	Q_INVOKABLE GFind(QObject* parent = nullptr);
	~GFind() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	QString heystack_;
	static QString printableStr(QString s);
	virtual void processFound(int itemIndex, int foundIndex, QString& foundStr);

public slots:
	void find(GPacket* packet);

signals:
	void found(GPacket* packet);
	void notFound(GPacket* packet);
};
