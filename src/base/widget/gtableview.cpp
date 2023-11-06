#include "gtableview.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QHeaderView>
#include <QScroller>

// ----------------------------------------------------------------------------
// GTableView
// ----------------------------------------------------------------------------
GTableView::GTableView(QWidget *parent) : QTableView(parent) {
	init();
}

void GTableView::init() {
	horizontalHeader()->setFixedHeight(GItemDelegate::DefaultItemHeight);
	QScroller::scroller(this)->grabGesture(this, QScroller::LeftMouseButtonGesture);
	setItemDelegate(&itemDelegate_);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

#endif
