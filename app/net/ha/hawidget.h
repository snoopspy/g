#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include "hostanalyzer.h"

struct G_EXPORT HaWidget : GDefaultWidget {
	Q_OBJECT

public:
	HaWidget(QWidget* parent = nullptr);
	~HaWidget() override;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
	void setControl();

public:
	GTreeWidget* treeWidget_{nullptr};
	HostAnalyzer hostAnalyzer_;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processProbeDetected(GMac mac, int signal);
	void processClosed();
};
