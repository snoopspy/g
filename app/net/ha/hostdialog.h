#pragma once

#include <QDialog>
#include <GHostDb>
#include <GProp>

struct HostDialog : QDialog, GProp {
	Q_OBJECT
public:
	explicit HostDialog(QWidget* parent, GHostDb* hostDb);
	~HostDialog();

protected:
	GHostDb* hostDb_;

public:
	QVBoxLayout* mainLayout_{nullptr};
	QTabWidget* tabWidget_{nullptr};

	// Host
	QWidget* hostWidget_{nullptr};
	QVBoxLayout* hostVLayout_{nullptr};
	QHBoxLayout* hostHLayout_{nullptr};
	QLineEdit* leSearchHost_{nullptr};
	QToolButton* tbSearchHost_{nullptr};
	QTableView* hostView_{nullptr};

	// Log
	QWidget* logWidget_{nullptr};
	QVBoxLayout* logVLayout_{nullptr};
	QHBoxLayout* logHLayout_{nullptr};
	QDateTimeEdit* dteBegin_{nullptr};
	QDateTimeEdit* dteEnd_{nullptr};
	QComboBox* cbPeriod_{nullptr};
	QLineEdit* leSearchLog_{nullptr};
	QToolButton* tbSearchLog_{nullptr};
	QTableView* logView_{nullptr};

	HostModel* hostModel_{nullptr};
	LogModel* logModel_{nullptr};

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
	const static int ColumnHostMac = 0;
	const static int ColumnHostIp = 1;
	const static int ColumnHostAlias = 2;
	const static int ColumnHostHost = 3;
	const static int ColumnHostVendor = 4;

	const static int ColumnLogName = 0;
	const static int ColumnLogIp = 1;
	const static int ColumnLogBegTime = 2;
	const static int ColumnLogEndTime = 3;
	const static int ColumnLogAlias = 4;
	const static int ColumnLogHost = 5;
	const static int ColumnLogVendor = 6;

	enum SearchPeriod {
		Min10,
		Min20,
		Min30,
		Hour1,
		Hour2,
		Hour3,
		Today,
		Yesterday,
		ThisWeek,
		LastWeek,
		ThisMonth,
		LastMonth,
		Custom
	};
	void setPeriod();

public slots:
	void tbSearchHost_clicked();
	void tbSearchLog_clicked();
	void cbPeriod_currentIndexChanged(int index);
};
