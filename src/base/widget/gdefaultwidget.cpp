#include "gdefaultwidget.h"
#include "base/sys/gsignal.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GDefaultWidget
// ----------------------------------------------------------------------------
GDefaultWidget::GDefaultWidget(QWidget* parent) : QWidget(parent) {
	resize(QSize(640, 480));

	mainLayout_ = new QVBoxLayout();
	toolButtonLayout_ = new QHBoxLayout();

	tbStart_ = new QToolButton(this);
	tbStart_->setText("Start");
	tbStart_->setToolTip("Start");
	tbStart_->setIcon(QIcon(":/img/start.png"));
	tbStart_->setAutoRaise(true);
	toolButtonLayout_->addWidget(tbStart_);

	tbStop_ = new QToolButton(this);
	tbStop_->setText("Stop");
	tbStop_->setToolTip("Stop");
	tbStop_->setIcon(QIcon(":/img/stop.png"));
	tbStop_->setAutoRaise(true);
	toolButtonLayout_->addWidget(tbStop_);

	tbOption_ = new QToolButton(this);
	tbOption_->setText("Option");
	tbOption_->setToolTip("Option");
	tbOption_->setIcon(QIcon(":/img/option.png"));
	tbOption_->setAutoRaise(true);
	toolButtonLayout_->addWidget(tbOption_);

	mainLayout_->addLayout(toolButtonLayout_);

	this->setLayout(mainLayout_);

#ifdef Q_OS_ANDROID
	tbStart_->setIconSize(QSize(160, 160));
#else
	tbStart_->setIconSize(QSize(32, 32));
#endif
	tbStop_->setIconSize(tbStart_->iconSize());
	tbOption_->setIconSize(tbStart_->iconSize());

#ifndef Q_OS_WIN
		GSignal& signal = GSignal::instance();
		QObject::connect(&signal, &GSignal::signaled, this, &GDefaultWidget::processSignal);
		signal.setupAll();
#endif // Q_OS_WIN
}

GDefaultWidget::~GDefaultWidget() {
}

void GDefaultWidget::processSignal(int signo) {
#ifdef Q_OS_WIN
		(void)signo;
#else // Q_OS_WIN
		QString str1 = GSignal::getString(signo);
		QString str2 = strsignal(signo);
		qWarning() << QString("signo=%1 signal=%2 msg=%3").arg(signo).arg(str1, str2);
		close();
#endif
}

#endif
