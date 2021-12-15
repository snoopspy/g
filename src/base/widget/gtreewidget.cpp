#include "gtreewidget.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QScroller>
#include <QStyledItemDelegate>

struct MyHeightItemDelegate : QStyledItemDelegate
{
	MyHeightItemDelegate(QObject *poParent = nullptr) : QStyledItemDelegate(poParent) {}

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override	{
		QSize res = QStyledItemDelegate::sizeHint(option, index);
		res.setHeight(res.height() * 3 / 2);
		return res;
	}
};

GTreeWidget::GTreeWidget(QWidget *parent) : QTreeWidget(parent) {
	setItemDelegate(new MyHeightItemDelegate(this));
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QScroller::scroller(this)->grabGesture(this, QScroller::LeftMouseButtonGesture);
}

#endif
