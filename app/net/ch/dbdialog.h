#pragma once

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <GTableView>
#include <GProp>
#include <GSplitter>

#include <GCookieHijack>

struct CookieModel;
struct DbDialog : QDialog, GProp {
	Q_OBJECT

public:
	explicit DbDialog(QWidget* parent, GCookieHijack* cookieHijack);
	~DbDialog() override;

protected:
	GCookieHijack* cookieHijack_;

public:
	QVBoxLayout* mainLayout_{nullptr};
	QTabWidget* tabWidget_{nullptr};

	// Cookie
	QWidget* cookieWidget_{nullptr};
	QVBoxLayout* cookieVLayout_{nullptr};
	QHBoxLayout* cookieHLayout_{nullptr};
	QDateTimeEdit* dteStart_{nullptr};
	QDateTimeEdit* dteEnd_{nullptr};
	QComboBox* cbPeriod_{nullptr};
	QLineEdit* leSearchCookie_{nullptr};
	QToolButton* tbSearchCookie_{nullptr};
	QToolButton* tbFirefox_{nullptr};
	GSplitter* splitter_{nullptr};
	QTableView* cookieView_{nullptr};
	QPlainTextEdit* plainTextEdit_{nullptr};

	CookieModel* cookieModel_{nullptr};

public:
	const static int ColumnCreated = 0;
	const static int ColumnMac = 1;
	const static int ColumnIp = 2;
	const static int ColumnHost = 3;
	const static int ColumnCookie = 4;

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
	void setControl();

public slots:
	void tbSearchLog_clicked();
	void cbPeriod_currentIndexChanged(int index);
	void doSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void tbFirefox_clicked();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
