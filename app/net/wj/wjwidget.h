#pragma once

#include <GDefaultWidget>
#include <GPlainTextEdit>
#include "wifijammer.h"

struct G_EXPORT WjWidget : GDefaultWidget {
	Q_OBJECT

public:
	WjWidget(QWidget* parent = nullptr);
	~WjWidget() override;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
	void setControl();

public:
	GPlainTextEdit* plainTextEdit_{nullptr};
	WifiJammer wifiJammer_;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processAttacked(GMac mac, QString ssid);
	void processClosed();
};
