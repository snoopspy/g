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

#include "base/widget/gitemdelegate.h"

// ----------------------------------------------------------------------------
// GTableWidget
// ----------------------------------------------------------------------------
#ifdef Q_OS_ANDROID
struct GTableWidget : QTableWidget {
	explicit GTableWidget(QWidget *parent = nullptr);
	GTableWidget(int rows, int columns, QWidget *parent = nullptr);

protected:
	void init();

public:
	GItemDelegate itemDelegate_{this};
};
#else
typedef QTableWidget GTableWidget;
#endif

#endif // QT_GUI_LIB
