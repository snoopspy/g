#pragma once

#include <QDialog>

#include "hostanalyzer.h"

struct DbDialog : QDialog, GProp {
public:
	explicit DbDialog(QWidget* parent = nullptr);
	~DbDialog();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
