#pragma once

#include <QProgressBar>
#include <QTimer>

#include <GDefaultWidget>
#include <GTableWidget>
#include "wifianalyzer.h"

struct Device {
	GMac mac_;
	QString ssid_;
	QProgressBar* progressBar_{nullptr};
	QVector<int> signals_;
};

struct Devices : QMap<GMac, Device> {
	QMutex m_;
};

struct G_EXPORT WaWidget : GDefaultWidget {
	Q_OBJECT

public:
	WaWidget(QWidget* parent = nullptr);
	~WaWidget() override;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
	void setControl();

public:
	GTableWidget* tableWidget_{nullptr};
	WifiAnalyzer wifiAnalyzer_;
	Devices devices_;
	QTimer* timer_{nullptr};

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processDetected(GMac mac, QString ssid, int signal);
	void processClosed();

	void updateDevices();
};
