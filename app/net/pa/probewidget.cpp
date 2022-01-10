#include "probewidget.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>

ProbeWidget::ProbeWidget(QWidget* parent) : QWidget(parent) {
	setWindowTitle("ProbeAnalyzer");

	QVBoxLayout* mainLayout = new QVBoxLayout();

	tableWidget_ = new QTableWidget(this);
	tableWidget_->setColumnCount(2);
	tableWidget_->setHorizontalHeaderItem(0, new QTableWidgetItem("Mac"));
	tableWidget_->setHorizontalHeaderItem(1, new QTableWidgetItem("Signal"));
	tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableWidget_->verticalHeader()->hide();

	mainLayout->addWidget(tableWidget_);

	QHBoxLayout* toolButtonLayout = new QHBoxLayout();

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

	QObject::connect(tbStart_, &QToolButton::clicked, this, &ProbeWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &ProbeWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &ProbeWidget::tbOption_clicked);
}

ProbeWidget::~ProbeWidget() {

}

void ProbeWidget::tbStart_clicked(bool checked) {
	(void)checked;
	qDebug() << "clicked";
}

void ProbeWidget::tbStop_clicked(bool checked) {
	(void)checked;
	qDebug() << "clicked";
}

void ProbeWidget::tbOption_clicked(bool checked) {
	(void)checked;
	qDebug() << "clicked";
}
