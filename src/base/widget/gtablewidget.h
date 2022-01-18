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

#include <QTableWidget>

#ifdef Q_OS_ANDROID
struct GTableWidget : QTableWidget {
	explicit GTableWidget(QWidget *parent = nullptr);
};
#else
typedef QTableWidget GTableWidget;
#endif

#endif // QT_GUI_LIB
