#pragma once

#include <QDialog>
#include <GHostDb>
#include <GProp>

struct HostDialog : QDialog, GProp {
	Q_OBJECT

public:
	HostDialog(QWidget* parent, GHostDb* hostDb, GMac mac);
	~HostDialog() override;

public:
	GHostDb* hostDb_;
	GMac mac_;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
