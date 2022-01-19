#pragma once

#include <GDefaultWidget>
#include <GPlainTextEdit>
#include "wifianalyzer.h"

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
	GPlainTextEdit* plainTextEdit_{nullptr};
	WifiAnalyzer wifiJammer_;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processAttacked(GMac mac, QString ssid);
	void processClosed();
};
