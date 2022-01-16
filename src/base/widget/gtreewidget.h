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

#ifdef Q_OS_ANDROID
struct GTreeWidget : QTreeWidget {
	explicit GTreeWidget(QWidget *parent = nullptr);
};
#else
typedef QTreeWidget GTreeWidget;
#endif

#endif // QT_GUI_LIB
