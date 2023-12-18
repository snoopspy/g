#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

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

	QPushButton* pbOk_{nullptr};
	QPushButton* pbCancel_{nullptr};

public slots:
	void setControl();
	void setControl2(int index);
	void pbOk_clicked();
	void pbCancel_clicked();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
