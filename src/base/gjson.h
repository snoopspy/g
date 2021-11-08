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

#include <QJsonObject>
#include "gbase.h"

// ----------------------------------------------------------------------------
// GJson
// ----------------------------------------------------------------------------
namespace GJson {
	QJsonObject G_EXPORT loadFromFile(QString fileName = "");
	bool G_EXPORT saveToFile(QJsonObject jo, QString fileName = "");
	QString G_EXPORT defaultFileName();
}

#include <QList>
// ----------------------------------------------------------------------------
// QList<int>
// ----------------------------------------------------------------------------
void G_EXPORT operator << (QJsonValueRef ref, const QList<int>& intList);
void G_EXPORT operator >> (const QJsonValue val, QList<int>& intList);

// ----------------------------------------------------------------------------
// GProp
// ----------------------------------------------------------------------------
struct GProp;
void G_EXPORT operator << (QJsonValueRef ref, const GProp& prop);
void G_EXPORT operator >> (const QJsonValue val, GProp& prop);

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GWidgetRect
// ----------------------------------------------------------------------------
#include <QWidget>
namespace GJson {
	struct GWidgetRect {
		GWidgetRect(QWidget* widget) { widget_ = widget; }
		QWidget* widget_;
	};
	GWidgetRect G_EXPORT rect(QWidget* widget);
}
void G_EXPORT operator << (QJsonValueRef ref, const GJson::GWidgetRect&& rect);
void G_EXPORT operator >> (const QJsonValue val, GJson::GWidgetRect&& rect);

// ----------------------------------------------------------------------------
// GSplitterSizes
// ----------------------------------------------------------------------------
#include <QSplitter>
namespace GJson {
	struct GSplitterSizes {
		GSplitterSizes(QSplitter* splitter) { splitter_ = splitter; }
		QSplitter* splitter_;
	};
	GSplitterSizes G_EXPORT splitterSizes(QSplitter* splitter);
}
void G_EXPORT operator << (QJsonValueRef ref, const GJson::GSplitterSizes&& sizes);
void G_EXPORT operator >> (const QJsonValue val, GJson::GSplitterSizes&& sizes);

// ----------------------------------------------------------------------------
// GTableWidgetColumnSizes
// ----------------------------------------------------------------------------
#include <QTableWidget>
namespace GJson {
	struct GTableWidgetColumnSizes {
		GTableWidgetColumnSizes(QTableWidget* tableWidget) { tableWidget_ = tableWidget; }
		QTableWidget* tableWidget_;
	};
	GTableWidgetColumnSizes G_EXPORT columnSizes(QTableWidget* tableView);
};
void G_EXPORT operator << (QJsonValueRef ref, const GJson::GTableWidgetColumnSizes&& columnSizes);
void G_EXPORT operator >> (const QJsonValue val, GJson::GTableWidgetColumnSizes&& columnSizes);

// ----------------------------------------------------------------------------
// GTreeViewColumnSizes
// ----------------------------------------------------------------------------
#include <QTreeView>
#include <QHeaderView>
namespace GJson {
	struct GTreeViewColumnSizes {
		GTreeViewColumnSizes(QTreeView* treeView) { treeView_ = treeView; }
		QTreeView* treeView_;
	};
	GTreeViewColumnSizes G_EXPORT columnSizes(QTreeView* treeView);
}
void G_EXPORT operator << (QJsonValueRef ref, const GJson::GTreeViewColumnSizes&& columnSizes);
void G_EXPORT operator >> (const QJsonValue val, GJson::GTreeViewColumnSizes&& columnSizes);


#endif // QT_GUI_LIB
