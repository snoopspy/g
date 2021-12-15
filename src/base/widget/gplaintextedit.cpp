#include "gplaintextedit.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QScrollBar>

struct MyScrollBar : public QScrollBar {
	MyScrollBar(QWidget * parent): QScrollBar(parent) {}

protected:
	QSize sizeHint() const override { return QSize(64, 64); }
	QSize minimumSizeHint() const override { return QSize(64, 64); }
};

GPlainTextEdit::GPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) {
	setVerticalScrollBar(new MyScrollBar(this->verticalScrollBar()));
}

GPlainTextEdit::GPlainTextEdit(const QString &text, QWidget *parent) : QPlainTextEdit(text, parent) {
	setVerticalScrollBar(new MyScrollBar(this->verticalScrollBar()));
}

#endif

