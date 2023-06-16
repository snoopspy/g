#pragma once

#include <GDefaultWidget>
#include <GTreeWidget>
#include "hostanalyzer.h"
#include "dbdialog.h"

struct ItemDelegate;
struct G_EXPORT HaWidget : GDefaultWidget, GProp {
	Q_OBJECT

public:
#ifdef Q_OS_ANDROID
	static const int ItemHeight = 140;
#else
	static const int ItemHeight = 32;
#endif // Q_OS_ANDROID

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

public:
	QToolButton* tbDb_{nullptr};
	DbDialog* dbDialog_{nullptr};

private slots:
	void tbStart_clicked(bool checked = false);
	void tbStop_clicked(bool checked = false);
	void tbOption_clicked(bool checked = false);
	void tbDb_clicked(bool checked = false);

	void processClosed();
};
