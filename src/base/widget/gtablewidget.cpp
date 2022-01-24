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

#include <QDebug>
GTableWidget::GTableWidget(QWidget *parent) : QTableWidget(parent) {
	setItemDelegate(new MyHeightItemDelegate(this));
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QScroller::scroller(this)->grabGesture(this, QScroller::LeftMouseButtonGesture);
	horizontalHeader()->setFixedHeight(75);
//	verticalHeader()->setDefaultSectionSize(verticalHeader()->defaultSectionSize() * 3 / 2); // gilgil temp 2022.01.25
}

#endif
