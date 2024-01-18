#pragma once

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <GProp>
#include "hostanalyzer.h"

struct HostDialog : QDialog, GProp {
	Q_OBJECT

public:
	HostDialog(QWidget* parent, GMac mac, HostAnalyzer* hostAnalyzer, GHostMgr::HostValue* hostValue);
	~HostDialog() override;

public:
	GMac mac_;
	HostAnalyzer* ha_;
	GHostMgr::HostValue* hv_;

protected:
	QLineEdit* leMac_{nullptr};
	QLineEdit* leIp_{nullptr};
	QLineEdit* leAlias_{nullptr};
	QLineEdit* leHost_{nullptr};
	QLineEdit* leVendor_{nullptr};
	QComboBox* cbMode_{nullptr};
	QDateTimeEdit* dteBlockTime_{nullptr};

	QPushButton* pbOk_{nullptr};
	QPushButton* pbCancel_{nullptr};

public:
	void setDateTimeEdit(bool showBlockTime);

public slots:
	void leAlias_TextChanged();
	void cdMode_currentIndexChanged(int index);
	void pbOk_clicked();
	void pbCancel_clicked();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
