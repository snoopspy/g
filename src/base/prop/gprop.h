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

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaObject>
#include <QMetaProperty>
#include "gpropitem.h"

#include "base/gbase.h"

// ----------------------------------------------------------------------------
// GProp
// ----------------------------------------------------------------------------
struct G_EXPORT GProp {
	virtual void propLoad(QJsonObject jo);
	virtual void propSave(QJsonObject& jo);

	virtual bool propLoad(QJsonObject jo, QMetaProperty mpro);
	virtual bool propSave(QJsonObject& jo, QMetaProperty mpro);

#ifdef QT_GUI_LIB
	virtual GPropItem* propCreateItem(GPropItemParam* param);
	virtual void propCreateItems(QTreeWidget* treeWidget, QTreeWidgetItem* parent, QObject* object);
#else
	virtual void* propCreateItem(void* param);
	virtual void propCreateItems(void* treeWidget, void* parent, QObject* object);
#endif

#ifdef QT_GUI_LIB
	static void showDialog(QObject* object);
#endif // QT_GUI_LIB
};
