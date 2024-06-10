#include "gtreewidget.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QHeaderView>
#include <QScroller>

#include "gitemdelegate.h"

// ----------------------------------------------------------------------------
// GTableWidget
// ----------------------------------------------------------------------------
GTreeWidget::GTreeWidget(QWidget *parent) : QTreeWidget(parent) {
	init();
	GItemDelegate* itemDelegate = new GItemDelegate(this);
	setItemDelegate(itemDelegate);
}

void GTreeWidget::init() {
	header()->setFixedHeight(GItemDelegate::DefaultItemHeight);
	setItemDelegate(&itemDelegate_);
	QScroller::scroller(this)->grabGesture(this, QScroller::LeftMouseButtonGesture);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

#endif
