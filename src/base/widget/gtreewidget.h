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

#ifdef QT_GUI_LIB

#include <QTreeWidget>

#include "gitemdelegate.h"

// ----------------------------------------------------------------------------
// QTreeWidget
// ----------------------------------------------------------------------------
#ifdef Q_OS_ANDROID
struct GTreeWidget : QTreeWidget {
	explicit GTreeWidget(QWidget *parent = nullptr);

protected:
	void init();

public:
	GItemDelegate itemDelegate_{this, GItemDelegate::DefaultItemHeight};
};
#else
typedef QTreeWidget GTreeWidget;
#endif

struct GTreeWidgetItem : QTreeWidgetItem, QObject {
	explicit GTreeWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
	explicit GTreeWidgetItem(const QStringList &strings, int type = Type) : QTreeWidgetItem(strings, type) {}
	explicit GTreeWidgetItem(QTreeWidget *treeview, int type = Type) : QTreeWidgetItem(treeview, type) {}
	GTreeWidgetItem(QTreeWidget *treeview, const QStringList &strings, int type = Type) : QTreeWidgetItem(treeview, strings, type) {}
	GTreeWidgetItem(QTreeWidget *treeview, QTreeWidgetItem *after, int type = Type) : QTreeWidgetItem(treeview, after, type) {}
	explicit GTreeWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
	GTreeWidgetItem(QTreeWidgetItem *parent, const QStringList &strings, int type = Type) : QTreeWidgetItem(parent, strings, type) {}
	GTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, int type = Type) : QTreeWidgetItem(parent, after, type) {}
	GTreeWidgetItem(const QTreeWidgetItem &other) : QTreeWidgetItem(other) {}
};
typedef GTreeWidgetItem *PTreeWidgetItem;
#endif // QT_GUI_LIB
