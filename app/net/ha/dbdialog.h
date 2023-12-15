#pragma once

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <GHostDb>
#include <GTableView>
#include <GProp>

struct HostModel;
struct LogModel;
struct DbDialog : QDialog, GProp {
	Q_OBJECT
public:
	explicit DbDialog(QWidget* parent, GHostDb* hostDb);
	~DbDialog() override;

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
	const static int ColumnHostMode = 5;

	const static int ColumnLogName = 0;
	const static int ColumnLogIp = 1;
	const static int ColumnLogSttTime = 2;
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
