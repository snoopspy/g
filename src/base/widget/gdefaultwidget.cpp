#include "gdefaultwidget.h"
#include "base/sys/gsignal.h"

#ifdef QT_GUI_LIB

GDefaultWidget::GDefaultWidget(QWidget* parent) : QWidget(parent) {
	resize(QSize(640, 480));

	mainLayout_ = new QVBoxLayout();
	toolButtonLayout_ = new QHBoxLayout();

	tbStart_ = new QToolButton(this);
	tbStart_->setText("Start");
	tbStart_->setIcon(QIcon(":/img/start.png"));
	tbStart_->setAutoRaise(true);
	toolButtonLayout_->addWidget(tbStart_);

	tbStop_ = new QToolButton(this);
	tbStop_->setText("Stop");
	tbStop_->setIcon(QIcon(":/img/stop.png"));
	tbStop_->setAutoRaise(true);
	toolButtonLayout_->addWidget(tbStop_);

	tbOption_ = new QToolButton(this);
	tbOption_->setText("Option");
	tbOption_->setIcon(QIcon(":/img/option.png"));
	tbOption_->setAutoRaise(true);
	toolButtonLayout_->addWidget(tbOption_);

	mainLayout_->addLayout(toolButtonLayout_);

	this->setLayout(mainLayout_);

#ifdef Q_OS_ANDROID
	QSize size(192, 192);
#else
	QSize size(32, 32);
#endif
	tbStart_->setIconSize(size);
	tbStop_->setIconSize(size);
	tbOption_->setIconSize(size);

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
