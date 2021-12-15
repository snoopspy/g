// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#include <QSplitter>

#ifdef Q_OS_ANDROID
struct GSplitter : QSplitter {
	explicit GSplitter(QWidget* parent = nullptr);
	explicit GSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);
};
#else
typedef QSplitter GSplitter;
#endif
