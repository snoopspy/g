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

#include <GMac>

struct HostModel : QSqlQueryModel {
	explicit HostModel(QObject *parent) : QSqlQueryModel(parent) {}
	Qt::ItemFlags flags(const QModelIndex &index) const override {
		Qt::ItemFlags res = QSqlQueryModel::flags(index);
		if (index.column() == 2) // alias
			res |= Qt::ItemIsEditable;
		return res;
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		QVariant res = QSqlQueryModel::data(index, role);
		if (role == Qt::DisplayRole) {
			if (index.column() == 0) { // mac
				GMac mac = res.toString();
				return QString(mac);
			}
		}
		return res;
	}
};

#include <hostdb.h>
struct LogModel : QSqlQueryModel {
	HostDb* hostDb_;
	explicit LogModel(QObject *parent, HostDb* hostDb) : QSqlQueryModel(parent), hostDb_(hostDb) {}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		QVariant res = QSqlQueryModel::data(index, role);
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
				case 0: { // mac
					GMac mac = res.toString();
					return hostDb_->getDefaultName(mac);
				}
				case 1: break; // ip
				case 2: // beg_time
				case 3: { // end_time
					QDateTime dt = QDateTime::fromSecsSinceEpoch(res.toULongLong());
					return dt.toString("MMdd hh:mm");
				}
			}
		}
		return res;
	}
};

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

	HostModel* hostModel_{nullptr};
	LogModel* logModel_{nullptr};

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;

public:
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
