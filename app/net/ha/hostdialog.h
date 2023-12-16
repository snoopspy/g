#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>

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

protected:
	QLineEdit* leMac_{nullptr};
	QLineEdit* leIp_{nullptr};
	QLineEdit* leAlias_{nullptr};
	QLineEdit* leHost_{nullptr};
	QLineEdit* leVendor_{nullptr};
	QComboBox* cbMode_{nullptr};

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
