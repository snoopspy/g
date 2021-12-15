// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#include <QTreeWidget>

#ifdef Q_OS_ANDROID
struct GTreeWidget : QTreeWidget {
	explicit GTreeWidget(QWidget *parent = nullptr);
};
#else
typedef QTreeWidget GTreeWidget;
#endif
