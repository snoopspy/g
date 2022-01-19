#include "wawidget.h"

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollBar>

#include <GJson>

WaWidget::WaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("WiFiAnalyzer");

	tableWidget_ = new GTableWidget(this);
	tableWidget_->setColumnCount(3);
	tableWidget_->setHorizontalHeaderItem(0, new QTableWidgetItem("Mac"));
	tableWidget_->setHorizontalHeaderItem(1, new QTableWidgetItem("SSID"));
	tableWidget_->setHorizontalHeaderItem(2, new QTableWidgetItem("Signal"));
	tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	tableWidget_->verticalHeader()->hide();
	mainLayout_->addWidget(tableWidget_);

	timer_ = new QTimer(this);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &WaWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &WaWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &WaWidget::tbOption_clicked);

	QObject::connect(&wifiAnalyzer_, &WifiAnalyzer::detected, this, &WaWidget::processDetected);
	QObject::connect(&wifiAnalyzer_.monitorDevice_, &GMonitorDevice::closed, this, &WaWidget::processClosed);

	QObject::connect(timer_, &QTimer::timeout, this, &WaWidget::updateDevices);

	setControl();
}

WaWidget::~WaWidget() {
	tbStop_->click();
	setControl();
}

void WaWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["pa"] >> wifiAnalyzer_;
}

void WaWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["pa"] << wifiAnalyzer_;
}

void WaWidget::setControl() {
	bool active = wifiAnalyzer_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
}

void WaWidget::tbStart_clicked(bool checked) {
	(void)checked;

	devices_.clear();
	tableWidget_->setRowCount(0);
	timer_->setInterval(wifiAnalyzer_.updateInterval_);
	timer_->start();

	if (!wifiAnalyzer_.open()) {
		QMessageBox::warning(this, "Error", wifiAnalyzer_.err->msg());
	}
	setControl();
}

void WaWidget::tbStop_clicked(bool checked) {
	(void)checked;

	wifiAnalyzer_.close();
	timer_->stop();
	setControl();
}

#include <GPropDialog>
void WaWidget::tbOption_clicked(bool checked) {
	(void)checked;

	GPropDialog propDialog;
	propDialog.widget_.setObject(&wifiAnalyzer_);

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

void WaWidget::processDetected(GMac mac, QString ssid, int signal) {
	//QString msg = QString("%1 %2 %3").arg(QString(mac)).arg(ssid).arg(signal);
	//qDebug() << msg;

	{
		QMutexLocker(&devices_.m_);
		Devices::iterator it = devices_.find(mac);
		if (it == devices_.end()) {
			Device item;
			item.mac_ = mac;
			item.ssid_ = ssid;
			it = devices_.insert(mac, item);
		}
		it->signals_.push_back(signal);
	}
}

void WaWidget::processClosed() {
	tbStop_->click();
}

void WaWidget::updateDevices() {
	QMutexLocker(&devices_.m_);
	qDebug() << "";
	qDebug() << "device count =" << devices_.count(); // gilgil temp 2022.01.19
	for (Devices::iterator it = devices_.begin(); it != devices_.end(); it++) {
		Device& device = it.value();
		qDebug() << QString("%1 %2").arg(device.ssid_).arg(device.signals_.count()); // gilgil temp 2022.01.19
		if (device.signals_.count() == 0) continue;

		if (device.progressBar_ == nullptr) {
			int row = tableWidget_->rowCount();
			tableWidget_->insertRow(row);

			QProgressBar* progressBar = device.progressBar_  = new QProgressBar(this);

			QLineEdit* lineEdit = new QLineEdit(this);
			lineEdit->setFrame(false);
			lineEdit->setText(QString(device.mac_));
			tableWidget_->setCellWidget(row, 0, lineEdit);

			tableWidget_->setCellWidget(row, 1, new QLabel(device.ssid_));

			progressBar->setMinimum(wifiAnalyzer_.minSignal_);
			progressBar->setMaximum(0);
			progressBar->setFormat("%v dBm");
			tableWidget_->setCellWidget(row, 2, progressBar);

			tableWidget_->scrollToBottom();
		}

		int value = 0;
		switch (wifiAnalyzer_.showType_) {
			case WifiAnalyzer::Average: {
				int total = 0;
				for (int i: device.signals_)
					total += i;
				value = total / device.signals_.count();
				break;
			}
			case WifiAnalyzer::Max: {
				int max = -999;
				for (int i: device.signals_)
					if (i > max) max = i;
				value = max;
				break;
			}
			case WifiAnalyzer::Min: {
				int min = 999;
				for (int i: device.signals_)
					if (i < min) min = i;
				value = min;
				break;
			}
		}

		device.progressBar_->setValue(value);
		device.signals_.clear();
		qDebug() << QString("%1 %2 %3").arg(QString(device.mac_)).arg(device.ssid_).arg(value);
	}
}
