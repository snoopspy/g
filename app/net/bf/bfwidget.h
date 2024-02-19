#pragma once

#include <GDefaultWidget>
#include <GPlainTextEdit>
#include "beaconflood.h"

struct G_EXPORT BfWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
	BfWidget(QWidget* parent = nullptr);
	~BfWidget() override;

public:
	void setControl();

public:
	GPlainTextEdit* plainTextEdit_{nullptr};
	BeaconFlood beaconFlood_;

public:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processClosed();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
