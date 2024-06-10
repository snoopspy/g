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

#include <QTableView>

#include "gitemdelegate.h"

// ----------------------------------------------------------------------------
// GTableView
// ----------------------------------------------------------------------------
#ifdef Q_OS_ANDROID
struct GTableView : QTableView {
	explicit GTableView(QWidget *parent = nullptr);

protected:
	void init();

public:
	GItemDelegate itemDelegate_{this};
};
#else
typedef QTableView GTableView;
#endif

#endif // QT_GUI_LIB
