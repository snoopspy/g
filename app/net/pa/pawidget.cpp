#include "pawidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>

PaWidget::PaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("ProbeAnalyzer");

	treeWidget_ = new GTreeWidget(this);
	treeWidget_->setColumnCount(ProbeAnalyzer::ColumnSignal + 1);
	treeWidget_->setHeaderLabels(QStringList{"Mac", "TY", "CH", "Signal"});
	mainLayout_->addWidget(treeWidget_);

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

	treeWidget_->clear();

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

	GPropDialog propDialog(this);
	propDialog.widget_.setObject(&probeAnalyzer_);

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("propDialog") == jo.end();
	jo["propDialog"] >> propDialog;

	propDialog.setModal(true);
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

void PaWidget::processProbeDetected(GMac mac, QString type, int channel, int signal) {
	qDebug() << QString(mac) << type << channel << signal;
	GTreeWidgetItem* twi = new GTreeWidgetItem(treeWidget_);
	twi->setText(ProbeAnalyzer::ColumnMac, QString(mac));
	twi->setText(ProbeAnalyzer::ColumnType, type);
	twi->setText(ProbeAnalyzer::ColumnChannel, QString::number(channel));

	QProgressBar* progressBar = new QProgressBar(this);
	progressBar->setMinimum(probeAnalyzer_.minSignal_);
	progressBar->setMaximum(0);
	progressBar->setFormat("%v dBm");
	progressBar->setValue(signal);
	treeWidget_->setItemWidget(twi, ProbeAnalyzer::ColumnSignal, progressBar);

	treeWidget_->addTopLevelItem(twi);
	treeWidget_->scrollToBottom();
}

void PaWidget::processClosed() {
	if (probeAnalyzer_.active())
		tbStop_->click();
}

void PaWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["pa"] >> probeAnalyzer_;
	jo["sizes"] >> GJson::columnSizes(treeWidget_);
}

void PaWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["pa"] << probeAnalyzer_;
	jo["sizes"] << GJson::columnSizes(treeWidget_);
}
