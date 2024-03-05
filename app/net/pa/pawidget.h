#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include "probeanalyzer.h"

struct G_EXPORT PaWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
	PaWidget(QWidget* parent = nullptr);
	~PaWidget() override;

public:
	void setControl();

public:
	GTreeWidget* treeWidget_{nullptr};
	ProbeAnalyzer probeAnalyzer_;

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
