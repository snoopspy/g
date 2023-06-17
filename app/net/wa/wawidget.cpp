#include "wawidget.h"

#include <QLabel>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollBar>

#include <GJson>

WaWidget::WaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("WiFi Analyzer");

	tableWidget_ = new GTableWidget(this);
	tableWidget_->setColumnCount(4);
	tableWidget_->setHorizontalHeaderItem(0, new QTableWidgetItem("Mac"));
	tableWidget_->setHorizontalHeaderItem(1, new QTableWidgetItem("SSID"));
	tableWidget_->setHorizontalHeaderItem(2, new QTableWidgetItem("CH"));
	tableWidget_->setHorizontalHeaderItem(3, new QTableWidgetItem("Signal"));
	tableWidget_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	tableWidget_->verticalHeader()->hide();
	tableWidget_->setSortingEnabled(true);
	mainLayout_->addWidget(tableWidget_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

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
	jo["sizes"] >> GJson::columnSizes(tableWidget_);
}

void WaWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["pa"] << wifiAnalyzer_;
	jo["sizes"] << GJson::columnSizes(tableWidget_);
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

	if (!wifiAnalyzer_.open()) {
		tbStop_->click();
		QMessageBox::warning(this, "Error", wifiAnalyzer_.err->msg());
		return;
	}

	timer_->setInterval(wifiAnalyzer_.updateInterval_);
	timer_->start();

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

void WaWidget::processDetected(GMac mac, QString ssid, int channel, int signal) {
	// qDebug() << QString("%1 %2 %3").arg(QString(mac)).arg(ssid).arg(signal); // gilgil temp 2022.01.19

	{
		QMutexLocker(&devices_.m_);
		Devices::iterator it = devices_.find(mac);
		if (it == devices_.end()) {
			Device item;
			item.mac_ = mac;
			item.ssid_ = ssid;
			item.channel_ = channel;
			it = devices_.insert(mac, item);
		}
		it->signals_.push_back(signal);
	}
}

void WaWidget::processClosed() {
	if (wifiAnalyzer_.active())
		tbStop_->click();
}

void WaWidget::updateDevices() {
	QMutexLocker(&devices_.m_);
	for (Devices::iterator it = devices_.begin(); it != devices_.end(); it++) {
		Device& device = it.value();
		if (device.signals_.count() == 0) continue;

		if (device.progressBar_ == nullptr) {
			tableWidget_->setSortingEnabled(false);

			int row = tableWidget_->rowCount();
			tableWidget_->insertRow(row);

			QProgressBar* progressBar = device.progressBar_ = new QProgressBar(this);

			tableWidget_->setItem(row, 0, new TextItem(QString(device.mac_)));
			tableWidget_->setItem(row, 1, new TextItem(device.ssid_));
			tableWidget_->setItem(row, 2, new TextItem(QString::number(device.channel_)));
			device.textItem_ = new TextItem("0");
			tableWidget_->setItem(row, 3, device.textItem_);


			progressBar->setMinimum(wifiAnalyzer_.minSignal_);
			progressBar->setMaximum(0);
			progressBar->setFormat("%v dBm");
			tableWidget_->setCellWidget(row, 3, progressBar);
			tableWidget_->setSortingEnabled(true);
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
		device.textItem_->setText(QString::number(value));
		device.signals_.clear();
		// qDebug() << QString("%1 %2 %3").arg(QString(device.mac_)).arg(device.ssid_).arg(value); // gilgil temp 2022.01.19
	}
}
