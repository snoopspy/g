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
#include <GTableView>
#include <GProp>

struct DbDialog : QDialog, GProp {
	Q_OBJECT
public:
	explicit DbDialog(QWidget* parent = nullptr);
	~DbDialog();

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

	QSqlQueryModel* hostModel_{nullptr};
	QSqlQueryModel* logModel_{nullptr};

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
	enum SearchPeriod {
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
