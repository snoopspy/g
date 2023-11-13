#include "gscreensaver.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// ScreenSaverWidget
// ----------------------------------------------------------------------------
ScreenSaverWidget::ScreenSaverWidget(GScreenSaver* screenSaver) : QWidget(nullptr), screenSaver_(screenSaver) {
	Q_ASSERT(screenSaver_ != nullptr);

	QPalette palette(QPalette::Window, Qt::black);
	setAutoFillBackground(true);
	setPalette(palette);

	label_ = new QLabel(this);
	timer_ = new QTimer(this);

	palette = label_->palette();
	palette.setColor(label_->foregroundRole(), QColor(screenSaver_->red_, screenSaver_->green_, screenSaver_->blue_));
	label_->setPalette(palette);

	updateCurrentTime();

	QObject::connect(timer_, &QTimer::timeout, this, &ScreenSaverWidget::updateCurrentTime);
	timer_->start(screenSaver_->updateInterval_);
}

ScreenSaverWidget::~ScreenSaverWidget() {
	qDebug() << "bef"; // gilgil temp 2023.11.14
	if (screenSaver_->active())
		screenSaver_->close();
	qDebug() << "end"; // gilgil temp 2023.11.14
}

void ScreenSaverWidget::closeEvent(QCloseEvent *event) {
	qDebug() << "beg"; // gilgil temp 2023.11.14
	QWidget::closeEvent(event);
	timer_->stop();
	qDebug() << "end"; // gilgil temp 2023.11.14
}

void ScreenSaverWidget::mousePressEvent(QMouseEvent *event)  {
	qDebug() << "beg"; // gilgil temp 2023.11.14
	QWidget::mousePressEvent(event);
	close();
	qDebug() << "end"; // gilgil temp 2023.11.14
}

void ScreenSaverWidget::updateCurrentTime() {
	QDateTime now = QDateTime::currentDateTime();
	QString msg = now.toString(screenSaver_->format_);
	label_->setText(msg);
	label_->adjustSize();

	QRect widgetRect = rect();
	QRect labelRect = label_->rect();
	qDebug() << widgetRect << labelRect;
	int left = std::rand() % (widgetRect.width() - labelRect.width());
	int top = std::rand() % (widgetRect.height() - labelRect.height());
	label_->setGeometry(QRect(left, top, labelRect.width(), labelRect.height()));
}

// ----------------------------------------------------------------------------
// GScreenSaver
// ----------------------------------------------------------------------------
bool GScreenSaver::doOpen() {
	qDebug() << ""; // gilgil temp 2023.11.13
	if (screenSaverWidget_ == nullptr) {
		screenSaverWidget_ = new ScreenSaverWidget(this);
	}

	switch (showStyle_) {
		case Show: screenSaverWidget_->show(); break;
		case ShowMaxized: screenSaverWidget_->showMaximized(); break;
		case ShowFullScreen: screenSaverWidget_->showFullScreen(); break;
	}

	return true;
}

bool GScreenSaver::doClose() {
	qDebug() << "beg"; // gilgil temp 2023.11.13
	if (screenSaverWidget_ != nullptr) {
		delete screenSaverWidget_;
		screenSaverWidget_ = nullptr;
	}

	qDebug() << "end"; // gilgil temp 2023.11.13
	return true;
}

#else // QT_GUI_LIB
bool GScreenSaver::doOpen() {
	qWarning() << "no nothing";
	return true;
}

bool GScreenSaver::doClose() {
	qWarning() << "no nothing";
	return true;
}
#endif // QT_GUI_LIB
