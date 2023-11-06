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
#include <QStyledItemDelegate>

struct GItemDelegate : QStyledItemDelegate {
protected:
	int height_{0};

public:
#ifdef Q_OS_ANDROID
	static const int DefaultItemHeight = 90;
#else
	static const int DefaultItemHeight = 32;
#endif // Q_OS_ANDROID

	GItemDelegate(QObject *parent, int height = 0) : QStyledItemDelegate(parent), height_(height) {
		height_ = height;
		if (height_ == 0)
			height_ = DefaultItemHeight;
	}

	int height() { return height_; }

	void setHeight(int height) { height_ = height; }

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override	{
		QSize res = QStyledItemDelegate::sizeHint(option, index);
		qDebug() << res << height_; // gilgil temp 2023.11.06
		if (height_ != 0)
			res.setHeight(height_);
		return res;
	}
};
