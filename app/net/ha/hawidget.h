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
	GTreeWidget* treeWidget_;
	HostAnalyzer hostAnalyzer_{this};

public:
	QToolButton* tbDb_{nullptr};
	QToolButton* tbEdit_{nullptr};
	QToolButton* tbQrCode_{nullptr};
	QToolButton* tbScreenSaver_{nullptr};

public:
	void closeEvent(QCloseEvent* event) override;

public slots:
	void setControl();

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);
	void tbDb_clicked(bool checked = false);
	void tbEdit_clicked(bool checked = false);
	void tbQrCode_clicked(bool checked = false);
	void tbScreenSaver_clicked(bool checked = false);

	void processClosed();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
