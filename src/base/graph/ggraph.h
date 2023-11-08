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
#include "base/gjson.h"

// ----------------------------------------------------------------------------
// GGraph
// ----------------------------------------------------------------------------
struct G_EXPORT GGraph : GStateObj {
	Q_OBJECT

public:
	// --------------------------------------------------------------------------
	// Nodes
	// --------------------------------------------------------------------------
	struct Nodes : public GObjList {
		Nodes(QObject* parent = nullptr) : GObjList(parent) {}
		~Nodes() override {}

		GObj* findNode(QString objectName);
	};

	// --------------------------------------------------------------------------
	// Connection
	// --------------------------------------------------------------------------
	struct Connection {
		GObj* sender_{nullptr};
		QString signal_{""};
		GObj* receiver_{nullptr};
		QString slot_{""};
	};

	// --------------------------------------------------------------------------
	// Connections
	// --------------------------------------------------------------------------
	struct Connections : QObject, QList<Connection*> {
		Connections(QObject* parent = nullptr) : QObject(parent) {}
		~Connections() override { clear(); }

		void clear();
		void load(QJsonArray ja);
		void save(QJsonArray& ja);
	};

	// --------------------------------------------------------------------------
	// Factory
	// --------------------------------------------------------------------------
	struct Factory : GObj {
		// ------------------------------------------------------------------------
		// Item
		// ------------------------------------------------------------------------
		struct Item {
			Item(QString displayName) : displayName_(displayName) {}
			virtual ~Item() {}
			QString displayName_; // class name or category name
		};
		typedef Item *PItem;

		// ------------------------------------------------------------------------
		// Items
		// ------------------------------------------------------------------------
		struct Items : QList<Item*> {
			virtual ~Items() {
				for (Item* item: *this)
					delete item;
			}
		};

		// ------------------------------------------------------------------------
		// ItemNode
		// ------------------------------------------------------------------------
		struct ItemNode : Item {
			ItemNode(QString className) : Item(className), className_(className) {}
			~ItemNode() override {}

			QString className_;
		};

		// ------------------------------------------------------------------------
		// ItemCategory
		// ------------------------------------------------------------------------
		struct ItemCategory : Item {
			ItemCategory(QString categoryName) : Item(categoryName), categoryName_(categoryName) {}
			~ItemCategory() override {}

			QString categoryName_;
			Items items_;
		};
		// ------------------------------------------------------------------------

	public:
		Factory(QObject* parent = nullptr);
		~Factory() override;

	public:
		Items items_;
	};
	// --------------------------------------------------------------------------

public:
	GGraph(QObject* parent = nullptr) : GStateObj(parent) {}
	~GGraph() override { close(); }

public:
	Factory* factory() { return factory_; }
	void setFactory(Factory* factory) { factory_ = factory; }

protected:
	Factory* factory_{nullptr};

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	Nodes nodes_{this};
	Connections connections_{this};
	bool closeGraphOnNodeClosed_{true};

public slots:
	void clear();
	void start();
	void stop();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
