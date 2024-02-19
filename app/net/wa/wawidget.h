#pragma once

#include <QProgressBar>
#include <QTimer>

#include <GDefaultWidget>
#include <GTableWidget>
#include "wifianalyzer.h"

struct TextItem: QTableWidgetItem
{
	explicit TextItem(QString text) : QTableWidgetItem(text) {}

	bool operator< (const QTableWidgetItem &r) const {
		int col = column();
		switch (col) {
			case 0: // mac
				return text() < r.text();
			case 1: // ssid
				return text() < r.text();
			case 2: // channel;
				return text().toInt() < r.text().toInt();
			case 3: // signal
				return text().toInt() < r.text().toInt();
			default:
				qWarning() << "unreachable code";
				return text() < r.text();
		}
	}
};

struct Device {
	GMac mac_;
	QString ssid_;
	int channel_;

	QProgressBar* progressBar_{nullptr};
	TextItem* textItem_{nullptr};
	QVector<int> signals_;
};

struct Devices : QMap<GMac, Device> {
	QMutex m_;
};

struct G_EXPORT WaWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
	WaWidget(QWidget* parent = nullptr);
	~WaWidget() override;

public:
	void setControl();

public:
	GTableWidget* tableWidget_{nullptr};
	WifiAnalyzer wifiAnalyzer_;
	Devices devices_;
	QTimer* timer_{nullptr};

public:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processDetected(GMac mac, QString ssid, int channel, int signal);
	void processClosed();

	void updateDevices();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
