#include "gsplitter.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

// ----------------------------------------------------------------------------
// GSplitter
// ----------------------------------------------------------------------------
GSplitter::GSplitter(QWidget* parent) : QSplitter(parent) {
	init();
}

GSplitter::GSplitter(Qt::Orientation orientation, QWidget* parent) : QSplitter(orientation, parent) {
	init();
}

void GSplitter::init() {
	setHandleWidth(handleWidth() * 5);
}

#endif

