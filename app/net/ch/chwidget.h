#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include "cookiehijack.h"

struct G_EXPORT ChWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
	ChWidget(QWidget* parent = nullptr);
	~ChWidget() override;

public:
	void setControl();

public:
	GTreeWidget* treeWidget_{nullptr};
	CookieHijack cookieHijack_;

public:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processProbeDetected(GMac mac, QString type, int channel, int signal);
	void processClosed();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
