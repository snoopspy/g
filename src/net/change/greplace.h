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
#include "net/process/gcorrectchecksum.h"

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

public:
	QString replaceHexPattern_;
};
typedef GReplaceItem *PReplaceItem;

// ----------------------------------------------------------------------------
// GReplace
// ----------------------------------------------------------------------------
struct G_EXPORT GReplace : GFind {
	Q_OBJECT

	GObjRefArrayPtr getItems() override { return &replaceItems_; }
	Q_PROPERTY(GObjRef correctChecksum READ getCorrectChecksum)

	GObjRef getCorrectChecksum() { return &correctChecksum_; }

public:
	bool log_{true};
	GObjRefArray<GReplaceItem> replaceItems_;
	GCorrectChecksum correctChecksum_;

public:
	Q_INVOKABLE GReplace(QObject* parent = nullptr) : GFind(parent) {}
	~GReplace() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	bool replaced_;
	void processFound(int itemIndex, int foundIndex, QString& foundStr) override;

public slots:
	void replace(GPacket* packet);

signals:
	void replaced(GPacket* packet);
	void notReplaced(GPacket* packet);
};
