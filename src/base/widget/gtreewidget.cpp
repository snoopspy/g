#include "gtreewidget.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QHeaderView>
#include <QScroller>

#include "base/widget/gitemdelegate.h"

// ----------------------------------------------------------------------------
// GTableWidget
// ----------------------------------------------------------------------------
GTreeWidget::GTreeWidget(QWidget *parent) : QTreeWidget(parent) {
	init();
}

void GTreeWidget::init() {
	header()->setFixedHeight(GItemDelegate::DefaultItemHeight);
	setItemDelegate(&itemDelegate_);
	QScroller::scroller(this)->grabGesture(this, QScroller::LeftMouseButtonGesture);
	setIndentation(indentation() * 3 / 2);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

#endif
