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

#include <QCheckBox>

// ----------------------------------------------------------------------------
// GCheckBox
// ----------------------------------------------------------------------------
#ifdef Q_OS_ANDROID
struct GCheckBox : QCheckBox {
	explicit GCheckBox(QWidget *parent = nullptr);
	explicit GCheckBox(const QString &text, QWidget *parent = nullptr);

protected:
	void init();
};
#else
typedef QCheckBox GCheckBox;
#endif

#endif // QT_GUI_LIB
