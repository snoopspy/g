#include "gsplitter.h"

#ifdef Q_OS_ANDROID

GSplitter::GSplitter(QWidget* parent) : QSplitter(parent) {
	setHandleWidth(handleWidth() * 5);
}

GSplitter::GSplitter(Qt::Orientation orientation, QWidget* parent) : QSplitter(orientation, parent) {
	setHandleWidth(handleWidth() * 5);
}

#endif
