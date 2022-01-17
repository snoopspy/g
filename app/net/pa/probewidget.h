#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QToolButton>

#include <GProp>
#include <GPropDialog>

#include "probeanalyzer.h"

struct G_EXPORT ProbeWidget : QWidget, GProp {
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
	QToolButton* tbStart_{nullptr};
	QToolButton* tbStop_{nullptr};
	QToolButton* tbOption_{nullptr};
	QTableWidget* tableWidget_{nullptr};
	ProbeAnalyzer probeAnalyzer_;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

private slots:
	void processSignal(int signo);
	void processProbeDetected(GMac mac, int8_t signal);
	void processClosed();
};
