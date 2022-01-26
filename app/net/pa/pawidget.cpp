#include "pawidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>

PaWidget::PaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("ProbeAnalyzer");

	tableWidget_ = new GTableWidget(this);
	tableWidget_->setColumnCount(2);
	tableWidget_->setHorizontalHeaderItem(0, new QTableWidgetItem("Mac"));
	tableWidget_->setHorizontalHeaderItem(1, new QTableWidgetItem("Signal"));
	tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableWidget_->verticalHeader()->hide();
	mainLayout_->addWidget(tableWidget_);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &PaWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &PaWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &PaWidget::tbOption_clicked);

	QObject::connect(&probeAnalyzer_, &ProbeAnalyzer::probeDetected, this, &PaWidget::processProbeDetected);
	QObject::connect(&probeAnalyzer_.monitorDevice_, &GMonitorDevice::closed, this, &PaWidget::processClosed);

	setControl();
}

PaWidget::~PaWidget() {
	tbStop_->click();
	setControl();
}

void PaWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["pa"] >> probeAnalyzer_;
}

void PaWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["pa"] << probeAnalyzer_;
}

void PaWidget::setControl() {
	bool active = probeAnalyzer_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
}

void PaWidget::tbStart_clicked(bool checked) {
	(void)checked;

	tableWidget_->setRowCount(0);

	if (!probeAnalyzer_.open()) {
		tbStop_->click();
		QMessageBox::warning(this, "Error", probeAnalyzer_.err->msg());
		return;
	}

	setControl();
}

void PaWidget::tbStop_clicked(bool checked) {
	(void)checked;

	probeAnalyzer_.close();
	setControl();
}

#include <GPropDialog>
void PaWidget::tbOption_clicked(bool checked) {
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

void PaWidget::processProbeDetected(GMac mac, int signal) {
	qDebug() << QString(mac) << signal;
	int row = tableWidget_->rowCount();
	tableWidget_->insertRow(row);

	QLineEdit* lineEdit = new QLineEdit(this);
	lineEdit->setFrame(false);
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

void PaWidget::processClosed() {
	if (probeAnalyzer_.active())
		tbStop_->click();
}
