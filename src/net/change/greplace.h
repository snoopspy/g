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

#include "net/change/gfind.h"

// ----------------------------------------------------------------------------
// GReplaceItem
// ----------------------------------------------------------------------------
struct GReplaceItem : GFindItem {
	Q_OBJECT
	Q_PROPERTY(QString replacePattern MEMBER replacePattern_)
	Q_PROPERTY(Type replaceType MEMBER replaceType_)
	Q_ENUMS(Type)

public:
	enum Type {
		String,
		HexValue,
	};

public:
	QString replacePattern_;
	Type replaceType_{String};
};
typedef GReplaceItem *PReplaceItem;

// ----------------------------------------------------------------------------
// GReplace
// ----------------------------------------------------------------------------
struct G_EXPORT GReplace : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool log MEMBER log_)
	Q_PROPERTY(GObjRefArrayPtr items READ getItems)

	GObjRefArrayPtr getItems() { return &items_; }

public:
	bool log_{true};
	GObjRefArray<GReplaceItem> items_;

public:
	Q_INVOKABLE GReplace(QObject* parent = nullptr);
	~GReplace() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public slots:
	void replace(GPacket* packet);

signals:
	void replaced(GPacket* packet);
	void notReplaced(GPacket* packet);
};
