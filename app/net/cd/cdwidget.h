#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include "cycledetect.h"

struct G_EXPORT CdWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
	CdWidget(QWidget* parent = nullptr);
	~CdWidget() override;

public:
	GTreeWidget* treeWidget_;
	CycleDetect cycleDetect_{this};

public:
	QToolButton* tbEdit_{nullptr};

public:
	void closeEvent(QCloseEvent* event) override;

public slots:
	void setControl();

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);
	void tbEdit_clicked(bool checked = false);

	void processClosed();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
