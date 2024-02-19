#include "pawidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>

PaWidget::PaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("ProbeAnalyzer");

	tableWidget_ = new GTableWidget(this);
#ifdef Q_OS_ANDROID
	tableWidget_->horizontalHeader()->setFixedHeight(GItemDelegate::DefaultItemHeight);
#endif // Q_OS_ANDROID
	tableWidget_->setColumnCount(2);
	tableWidget_->setHorizontalHeaderItem(0, new QTableWidgetItem("Mac"));
	tableWidget_->setHorizontalHeaderItem(1, new QTableWidgetItem("Signal"));
	tableWidget_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	tableWidget_->verticalHeader()->hide();
	mainLayout_->addWidget(tableWidget_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

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
	jo["sizes"] >> GJson::columnSizes(tableWidget_);
}

void PaWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["pa"] << probeAnalyzer_;
	jo["sizes"] << GJson::columnSizes(tableWidget_);
}

void PaWidget::setControl() {
	bool active = probeAnalyzer_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
}

#include <QCloseEvent>
void PaWidget::closeEvent(QCloseEvent* event) {
#ifdef Q_OS_ANDROID
	QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm", "Are you sure want to exit?", QMessageBox::Yes | QMessageBox::No);
	qDebug() << "qCloseEvent" << reply;
	if (reply == QMessageBox::Yes)
		event->accept();
	else
		event->ignore();
#else
	event->accept();
#endif // Q_OS_ANDROID
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
	bool isFirst = jo.find("propDialog") == jo.end();
	jo["propDialog"] >> propDialog;

#ifdef Q_OS_ANDROID
	propDialog.showMaximized();
#else
	propDialog.show();
#endif
	if (isFirst) {
		int width = propDialog.width();
		propDialog.widget_.treeWidget_->setColumnWidth(0, width / 2);
	}

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
