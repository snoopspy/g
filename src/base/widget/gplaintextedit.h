// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#include <QPlainTextEdit>

#ifdef Q_OS_ANDROID
struct GPlainTextEdit : QPlainTextEdit {
	explicit GPlainTextEdit(QWidget *parent = nullptr);
	explicit GPlainTextEdit(const QString &text, QWidget *parent = nullptr);
};
#else
typedef QPlainTextEdit GPlainTextEdit;
#endif
