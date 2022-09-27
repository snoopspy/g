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

#include <QLibrary>
#include <QList>
#include "ggraph.h"

// ----------------------------------------------------------------------------
// GPluginFactory
// ----------------------------------------------------------------------------
struct G_EXPORT GPluginFactory : GGraph::Factory {
	GPluginFactory(QObject* parent = nullptr);
	~GPluginFactory() override;

public:
	void loadDefault();

protected:
	void loadBlock();
	void loadCapture();
	void loadConvert();
	void loadDelay();
	void loadFilter();
	void loadManage();
	void loadProcess();
	void loadWrite();

public:
	void load(QString folder);

protected:
	void loadFile(GGraph::Factory::ItemCategory* category, QString fileName);
	void loadFolder(GGraph::Factory::ItemCategory* category, QString folder);

protected:
	QList<QLibrary*> libraries_;

public:
	static GPluginFactory& instance() {
		static GPluginFactory pluginFactory;
		return pluginFactory;
	}
};
