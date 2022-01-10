#include "probewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

ProbeWidget::ProbeWidget(QWidget* parent) : QWidget(parent) {
	setWindowTitle("ProbeAnalyzer");

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	tableWidget_ = new QTableWidget(this);
	mainLayout->addWidget(tableWidget_);

	QHBoxLayout* toolButtonLayout = new QHBoxLayout(this);

	tbStart_ = new QToolButton(this);
	tbStart_->setText("Start");
	tbStart_->setIcon(QIcon(":/img/start.png"));
	tbStart_->setAutoRaise(true);
	toolButtonLayout->addWidget(tbStart_);

	tbStop_ = new QToolButton(this);
	tbStop_->setText("Stop");
	tbStop_->setIcon(QIcon(":/img/stop.png"));
	tbStop_->setAutoRaise(true);
	toolButtonLayout->addWidget(tbStop_);

	tbOption_ = new QToolButton(this);
	tbOption_->setText("Option");
	tbOption_->setIcon(QIcon(":/img/option.png"));
	tbOption_->setAutoRaise(true);
	toolButtonLayout->addWidget(tbOption_);

	mainLayout->addLayout(toolButtonLayout);

	this->setLayout(mainLayout);

#ifdef Q_OS_ANDROID
	QSize size(192, 192);
#else
	QSize size(32, 32);
#endif
	tbStart_->setIconSize(size);
	tbStop_->setIconSize(size);
	tbOption_->setIconSize(size);
}

ProbeWidget::~ProbeWidget() {

}
