#include "gcheckbox.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QStyle>

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
	setStyleSheet("QCheckBox { color : #AAAAAA; }\nQCheckBox::indicator { width : 65px; height : 65px; border : }");
}

#endif
