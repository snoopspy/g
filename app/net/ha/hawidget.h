#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include "hostanalyzer.h"

struct ItemDelegate;
struct G_EXPORT HaWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
	static const int ItemHeight = 50;

public:
	HaWidget(QWidget* parent = nullptr);
	~HaWidget() override;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
	void setControl();

public:
	ItemDelegate* itemDelegate_;
	GTreeWidget* treeWidget_;
	HostAnalyzer hostAnalyzer_;

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);

	void processClosed();
};
