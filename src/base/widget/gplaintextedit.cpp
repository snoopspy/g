#include "gplaintextedit.h"

#if defined(QT_GUI_LIB) && defined(Q_OS_ANDROID)

#include <QScrollBar>

struct MyScrollBar : public QScrollBar {
	MyScrollBar(QWidget * parent): QScrollBar(parent) {}

protected:
	QSize sizeHint() const override {
		return QSize(20, 20);
	}
	QSize minimumSizeHint() const override {
		return QSize(20, 20);
	}
};

// ----------------------------------------------------------------------------
// GPlainTextEdit
// ----------------------------------------------------------------------------
GPlainTextEdit::GPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) {
	init();
}

GPlainTextEdit::GPlainTextEdit(const QString &text, QWidget *parent) : QPlainTextEdit(text, parent) {
	init();
}

void GPlainTextEdit::init() {
	setVerticalScrollBar(new MyScrollBar(this->verticalScrollBar()));
}

#endif
