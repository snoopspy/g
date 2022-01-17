#pragma once

#include <GDefaultWidget>
#include <QTableWidget>
#include "probeanalyzer.h"

struct G_EXPORT ProbeWidget : GDefaultWidget {
	Q_OBJECT

public:
	ProbeWidget(QWidget* parent = nullptr);
	~ProbeWidget() override;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
	void setControl();

public:
	QTableWidget* tableWidget_{nullptr};
	ProbeAnalyzer probeAnalyzer_;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processProbeDetected(GMac mac, int8_t signal);
	void processClosed();
};
