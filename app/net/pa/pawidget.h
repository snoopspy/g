#pragma once

#include <GDefaultWidget>
#include <GTableWidget>
#include "probeanalyzer.h"

struct G_EXPORT PaWidget : GDefaultWidget {
	Q_OBJECT

public:
	PaWidget(QWidget* parent = nullptr);
	~PaWidget() override;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
	void setControl();

public:
	GTableWidget* tableWidget_{nullptr};
	ProbeAnalyzer probeAnalyzer_;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processProbeDetected(GMac mac, int signal);
	void processClosed();
};
