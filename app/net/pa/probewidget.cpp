#include "probewidget.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QVBoxLayout>

#include <GJson>
#include <GSignal>

ProbeWidget::ProbeWidget(QWidget* parent) : QWidget(parent) {
	resize(QSize(640, 480));
	setWindowTitle("ProbeAnalyzer");

	QVBoxLayout* mainLayout = new QVBoxLayout();

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

	tableWidget_ = new QTableWidget(this);
	tableWidget_->setColumnCount(2);
	tableWidget_->setHorizontalHeaderItem(0, new QTableWidgetItem("Mac"));
	tableWidget_->setHorizontalHeaderItem(1, new QTableWidgetItem("Signal"));
	tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableWidget_->verticalHeader()->hide();

	mainLayout->addWidget(tableWidget_);

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
	//QObject::connect(&probeAnalyzer_, &ProbeAnalyzer::probeDetected, this, &ProbeWidget::processProbeDetected, Qt::BlockingQueuedConnection);
	QObject::connect(&probeAnalyzer_, &ProbeAnalyzer::probeDetected, this, &ProbeWidget::processProbeDetected);

#ifndef Q_OS_WIN
		GSignal& signal = GSignal::instance();
		QObject::connect(&signal, &GSignal::signaled, this, &ProbeWidget::processSignal);
		signal.setupAll();
#endif // Q_OS_WIN

	setControl();
}

ProbeWidget::~ProbeWidget() {
	tbStop_->click();
	setControl();
}

void ProbeWidget::propLoad(QJsonObject jo) {
	jo["pa"] >> probeAnalyzer_;
	jo["rect"] >> GJson::rect(this);

}

void ProbeWidget::propSave(QJsonObject& jo) {
	jo["pa"] << probeAnalyzer_;
	jo["rect"] << GJson::rect(this);
}

void ProbeWidget::setControl() {
	bool active = probeAnalyzer_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
}

void ProbeWidget::processSignal(int signo) {
#ifdef Q_OS_WIN
		(void)signo;
#else // Q_OS_WIN
		QString str1 = GSignal::getString(signo);
		QString str2 = strsignal(signo);
		qWarning() << QString("signo=%1 signal=%2 msg=%3").arg(signo).arg(str1, str2);
		close();
#endif
}

void ProbeWidget::processProbeDetected(GMac mac, int8_t signal) {
	qDebug() << QString(mac) << signal;
	int row = tableWidget_->rowCount();
	tableWidget_->insertRow(row);

	QLineEdit* lineEdit = new QLineEdit(this);
	lineEdit->setText(QString(mac));
	tableWidget_->setCellWidget(row, 0, lineEdit);

	QProgressBar* progressBar = new QProgressBar(this);
	progressBar->setMinimum(probeAnalyzer_.minSignal_);
	progressBar->setMaximum(0);
	progressBar->setFormat("%v dBm");
	progressBar->setValue(signal);
	tableWidget_->setCellWidget(row, 1, progressBar);

	tableWidget_->scrollToBottom();
}

void ProbeWidget::tbStart_clicked(bool checked) {
	(void)checked;

	tableWidget_->setRowCount(0);

	if (!probeAnalyzer_.open()) {
		QMessageBox::warning(this, "Error", probeAnalyzer_.err->msg());
	}
	setControl();
}

void ProbeWidget::tbStop_clicked(bool checked) {
	(void)checked;

	probeAnalyzer_.close();
	setControl();
}

void ProbeWidget::tbOption_clicked(bool checked) {
	(void)checked;

	GPropDialog propDialog;
	propDialog.widget_.setObject(&probeAnalyzer_);

	QJsonObject& jo = GJson::instance();
	jo["propDialog"] >> propDialog;

#ifndef Q_OS_ANDROID
	propDialog.show();
#else
	propDialog.showMaximized();
#endif
	propDialog.exec();

	jo["propDialog"] << propDialog;
}
