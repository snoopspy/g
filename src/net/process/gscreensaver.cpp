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
	label_->setVisible(false);
	updateTimer_ = new QTimer(this);
	setLabelVisibleTimer_ = new QTimer(this);

	palette = label_->palette();
	palette.setColor(label_->foregroundRole(), QColor(screenSaver_->red_, screenSaver_->green_, screenSaver_->blue_));
	label_->setPalette(palette);

	if (screenSaver_->fontSize_ != 0) {
		QFont font = label_->font();
		font.setPointSize(screenSaver_->fontSize_);
		label_->setFont(font);
	}

	QObject::connect(updateTimer_, &QTimer::timeout, this, &ScreenSaverWidget::updateCurrentTime);
	QObject::connect(setLabelVisibleTimer_, &QTimer::timeout, this, &ScreenSaverWidget::setLabelVisible);
	updateTimer_->start(screenSaver_->updateInterval_);
	setLabelVisibleTimer_->start(1000);
}

ScreenSaverWidget::~ScreenSaverWidget() {
	if (screenSaver_->active())
		screenSaver_->close();
}

void ScreenSaverWidget::mousePressEvent(QMouseEvent *event)  {
	QWidget::mousePressEvent(event);
	close();
}

void ScreenSaverWidget::paintEvent(QPaintEvent *event) {
	QWidget::paintEvent(event);
	if (rect() != lastRect_) {
		lastRect_ = rect();
		updateCurrentTime();
	}
}

void ScreenSaverWidget::closeEvent(QCloseEvent *event) {
	updateTimer_->stop();
	QWidget::closeEvent(event);
}

void ScreenSaverWidget::keyPressEvent(QKeyEvent *event) {
	QWidget::keyPressEvent(event);
	close();
}

void ScreenSaverWidget::updateCurrentTime() {
	QDateTime now = QDateTime::currentDateTime();
	QString msg = now.toString(screenSaver_->format_);
	label_->setText(msg);
	label_->adjustSize();

	QRect widgetRect = rect();
	QRect labelRect = label_->rect();
	int left = std::rand() % (widgetRect.width() - labelRect.width());
	int top = std::rand() % (widgetRect.height() - labelRect.height());
	label_->setGeometry(QRect(left, top, labelRect.width(), labelRect.height()));
}

void ScreenSaverWidget::setLabelVisible() {
	label_->setVisible(true);
	setLabelVisibleTimer_->stop();
}

// ----------------------------------------------------------------------------
// GScreenSaver
// ----------------------------------------------------------------------------
bool GScreenSaver::doOpen() {
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
	if (screenSaverWidget_ != nullptr) {
		delete screenSaverWidget_;
		screenSaverWidget_ = nullptr;
	}

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
