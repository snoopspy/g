#include "gtablewidget.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QHeaderView>
#include <QScroller>

#include "gitemdelegate.h"

// ----------------------------------------------------------------------------
// GTableWidget
// ----------------------------------------------------------------------------
GTableWidget::GTableWidget(QWidget *parent) : QTableWidget(parent) {
	init();
}

GTableWidget::GTableWidget(int rows, int columns, QWidget *parent) : QTableWidget(rows, columns, parent) {
	init();
}

void GTableWidget::init() {
	horizontalHeader()->setFixedHeight(GItemDelegate::DefaultItemHeight);
	QScroller::scroller(this)->grabGesture(this, QScroller::LeftMouseButtonGesture);
	setItemDelegate(&itemDelegate_);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

#endif
