#include "gcheckbox.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

// ----------------------------------------------------------------------------
// GCheckBox
// ----------------------------------------------------------------------------
GCheckBox::GCheckBox(QWidget *parent) : QCheckBox(parent) {
	init();
}

GCheckBox::GCheckBox(const QString &text, QWidget *parent) : QCheckBox(text, parent) {
	init();
}

void GCheckBox::init() {
	setStyleSheet("QCheckBox::indicator {width:50px;height: 50px;}");
}

#endif
