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

#include <QSplitter>

// ----------------------------------------------------------------------------
// GSplitter
// ----------------------------------------------------------------------------
#ifdef Q_OS_ANDROID
struct GSplitter : QSplitter {
	explicit GSplitter(QWidget* parent = nullptr);
	explicit GSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
	void init();
};
#else
typedef QSplitter GSplitter;
#endif

#endif // QT_GUI_LIB
