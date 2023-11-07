// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include <QDebug>
#include <QItemDelegate>

#ifdef Q_OS_ANDROID

struct GItemDelegate : QItemDelegate {
protected:
	int height_{0};

public:
	static const int DefaultItemHeight = 90;

	GItemDelegate(QObject *parent, int height = 0) : QItemDelegate(parent), height_(height) {
		height_ = height;
		if (height_ == 0)
			height_ = DefaultItemHeight;
	}

	int height() { return height_; }

	void setHeight(int height) { height_ = height; }

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override	{
		QSize res = QItemDelegate::sizeHint(option, index);
		if (height_ != 0)
			res.setHeight(height_);
		return res;
	}
};
#else
typedef QItemDelegate GItemDelegate;
#endif // Q_OS_ANDROID
