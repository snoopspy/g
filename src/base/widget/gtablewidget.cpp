#include "gtablewidget.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QHeaderView>
#include <QScroller>
#include <QStyledItemDelegate>

struct MyHeightItemDelegate : QStyledItemDelegate
{
	MyHeightItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override	{
		QSize res = QStyledItemDelegate::sizeHint(option, index);
		res.setHeight(res.height() * 3 / 2);
		return res;
	}
};

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
	horizontalHeader()->setFixedHeight(80);
	QScroller::scroller(this)->grabGesture(this, QScroller::LeftMouseButtonGesture);
	setItemDelegate(new MyHeightItemDelegate(this));
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

#endif
