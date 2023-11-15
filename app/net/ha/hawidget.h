#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include "hostanalyzer.h"

struct G_EXPORT HaWidget : GDefaultWidget, GProp {
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
	GTreeWidget* treeWidget_;
	HostAnalyzer hostAnalyzer_{this};

public:
	QToolButton* tbDb_{nullptr};
	QToolButton* tbScreenSaver_{nullptr};

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);
	void tbDb_clicked(bool checked = false);
	void tbScreenSaver_clicked(bool checked = false);

	void processClosed();
};
