#include "dbdialog.h"

#include <QMessageBox>
#include <QSqlTableModel>
#include <GMac>
#include <GIp>
#include <GJson>

#include <hostdb.h>
struct HostModel : QSqlQueryModel {
	HostDb* hostDb_;
	explicit HostModel(QObject *parent, HostDb* hostDb) : QSqlQueryModel(parent), hostDb_(hostDb) {}

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

	bool setData(const QModelIndex &index, const QVariant &value, int role) override {
		if (index.column() == DbDialog::ColumnHostAlias && role == Qt::EditRole) {
			QModelIndex macIndex = index.siblingAtColumn(DbDialog::ColumnHostMac);
			GMac mac = macIndex.data().toString();
			QString alias = value.toString();
			bool res = hostDb_->updateAlias(mac, alias);
			DbDialog* dbDialog = dynamic_cast<DbDialog*>(parent());
			Q_ASSERT(dbDialog != nullptr);
			dbDialog->tbSearchHost_->click();
			return res;
		}
		return QSqlQueryModel::setData(index, value, role);
	}
};

struct LogModel : QSqlQueryModel {
	HostDb* hostDb_;
	explicit LogModel(QObject *parent, HostDb* hostDb) : QSqlQueryModel(parent), hostDb_(hostDb) {}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		QVariant res = QSqlQueryModel::data(index, role);
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
				case DbDialog::ColumnLogName: {
					GMac mac = res.toString();
					return hostDb_->getDefaultName(mac);
				}
				case DbDialog::ColumnLogIp: break;
				case DbDialog::ColumnLogBegTime:
				case DbDialog::ColumnLogEndTime: {
					QDateTime dt = QDateTime::fromSecsSinceEpoch(res.toULongLong());
					return dt.toString("MMdd hh:mm");
				}
			}
		}
		return res;
	}
};


DbDialog::DbDialog(QWidget* parent) : QDialog(parent) {
	resize(QSize(640, 480));
	setWindowTitle("Database");

	mainLayout_ = new QVBoxLayout;
	mainLayout_->setContentsMargins(0, 0, 0, 0);
	mainLayout_->setSpacing(0);
	tabWidget_ = new QTabWidget(this);

	//
	// Host
	//
	hostWidget_ = new QWidget(this);
		hostVLayout_ = new QVBoxLayout;
		hostVLayout_->setContentsMargins(0, 0, 0, 0);
		hostVLayout_->setSpacing(0);
		hostWidget_->setLayout(hostVLayout_);
			hostHLayout_ = new QHBoxLayout;
			hostHLayout_->setContentsMargins(0, 0, 0, 0);
			hostVLayout_->setSpacing(0);
				leSearchHost_ = new QLineEdit(this);
				hostHLayout_->addWidget(leSearchHost_);
				tbSearchHost_ = new QToolButton(this);
				tbSearchHost_->setText("DB");
				tbSearchHost_->setIcon(QIcon(":/img/search.png"));
				tbSearchHost_->setAutoRaise(true);
				hostHLayout_->addWidget(tbSearchHost_);
			hostView_ = new GTableView(this);
			hostView_->verticalHeader()->hide();
			hostView_->setEditTriggers(QAbstractItemView::AllEditTriggers);
		hostVLayout_->addLayout(hostHLayout_);
		hostVLayout_->addWidget(hostView_);

	//
	// Log
	//
	logWidget_ = new QWidget(this);
		logVLayout_ = new QVBoxLayout;
		logVLayout_->setContentsMargins(0, 0, 0, 0);
		logVLayout_->setSpacing(0);
		logWidget_->setLayout(logVLayout_);
			logHLayout_ = new QHBoxLayout;
			logHLayout_->setContentsMargins(0, 0, 0, 0);
			logVLayout_->setSpacing(0);
				dteBegin_ = new QDateTimeEdit(this);
				dteBegin_->setDisplayFormat("yy/MM/dd hh:mm");
				logHLayout_->addWidget(dteBegin_);
				dteEnd_ = new QDateTimeEdit(this);
				dteEnd_->setDisplayFormat("yy/MM/dd hh:mm");
				logHLayout_->addWidget(dteEnd_);
				cbPeriod_ = new QComboBox(this);
				cbPeriod_->addItems(QStringList{"10 Minutes", "20 Minutes", "30 Minutes", "1 Hour", "2 Hours", "3 Hours", "Today", "Yesterday", "This Week", "Last Week", "This Month", "Last Month", "Custom"});
				cbPeriod_->setCurrentIndex(int(Today));
				logHLayout_->addWidget(cbPeriod_);
				leSearchLog_ = new QLineEdit(this);
				logHLayout_->addWidget(leSearchLog_);
				tbSearchLog_ = new QToolButton(this);
				tbSearchLog_->setText("DB");
				tbSearchLog_->setIcon(QIcon(":/img/search.png"));
				tbSearchLog_->setAutoRaise(true);
				logHLayout_->addWidget(tbSearchLog_);
			logView_ = new GTableView(this);
			logView_->verticalHeader()->hide();
		logVLayout_->addLayout(logHLayout_);
		logVLayout_->addWidget(logView_);

	tabWidget_->addTab(hostWidget_, "Host");
	tabWidget_->addTab(logWidget_, "Log");
	mainLayout_->addWidget(tabWidget_);
	this->setLayout(mainLayout_);

	QObject::connect(tbSearchHost_, &QToolButton::clicked, this, &DbDialog::tbSearchHost_clicked);
	QObject::connect(tbSearchLog_, &QToolButton::clicked, this, &DbDialog::tbSearchLog_clicked);
	QObject::connect(cbPeriod_, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPeriod_currentIndexChanged(int)));
	QObject::connect(leSearchHost_, &QLineEdit::returnPressed, this, &DbDialog::tbSearchHost_clicked);
	QObject::connect(leSearchLog_, &QLineEdit::returnPressed, this, &DbDialog::tbSearchLog_clicked);
}

DbDialog::~DbDialog() {
}

void DbDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	tabWidget_->setCurrentIndex(jo["tabIndex"].toInt(0));
	leSearchHost_->setText(jo["searchHost"].toString());
	dteBegin_->setDateTime(QDateTime::fromString(jo["begTime"].toString(), "yy/MM/dd hh:mm"));
	dteEnd_->setDateTime(QDateTime::fromString(jo["endTime"].toString(), "yy/MM/dd hh:mm"));
	leSearchLog_->setText(jo["searchLog"].toString());
	cbPeriod_->setCurrentIndex(jo["searchPeriod"].toInt(int(Today)));
}

void DbDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["tabIndex"] = tabWidget_->currentIndex();
	jo["searchHost"] = leSearchHost_->text();
	jo["begTime"] = dteBegin_->dateTime().toString("yy/MM/dd hh:mm");
	jo["endTime"] = dteEnd_->dateTime().toString("yy/MM/dd hh:mm");
	jo["searchLog"] = leSearchLog_->text();
	jo["searchPeriod"] = cbPeriod_->currentIndex();
}

void DbDialog::setPeriod() {
	SearchPeriod period = SearchPeriod(cbPeriod_->currentIndex());
	QDateTime begTime = QDateTime::currentDateTime();
	QDateTime endTime = begTime;
	switch (period) {
		case Min10:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 600); // 10 minutes
			break;
		case Min20:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 1200); // 20 minutes
			break;
		case Min30:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 1800); // 30 minutes
			break;
		case Hour1:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600); // 1 Hour
			break;
		case Hour2:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600 * 2); // 2 Hours
			break;
		case Hour3:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600 * 3); // 3 Hours
			break;
		case Today:
			begTime.setTime(QTime(0, 0));
			break;
		case Yesterday: {
			begTime.setDate(begTime.addDays(-1).date());
			begTime.setTime(QTime(0, 0));
			endTime.setDate(endTime.addDays(-1).date());
			endTime.setTime(QTime(23, 59));
			break;
		}
		case ThisWeek:
			while(begTime.date().dayOfWeek() != 7) { // Sunday
				begTime.setDate(begTime.addDays(-1).date());
			}
			begTime.setTime(QTime(0, 0));
			break;
		case LastWeek:
			begTime.setDate(begTime.addDays(-7).date());
			while(begTime.date().dayOfWeek() != 7) { // Sunday
				begTime.setDate(begTime.addDays(-1).date());
			}
			begTime.setTime(QTime(0, 0));
			endTime.setDate(endTime.addDays(-7).date());
			while(endTime.date().dayOfWeek() != 6) { // Saturday
				endTime.setDate(endTime.addDays(1).date());
			}
			endTime.setTime(QTime(23, 59));
			break;
		case ThisMonth:
			begTime.setDate(QDate(begTime.date().year(), begTime.date().month(), 1));
			begTime.setTime(QTime(0, 0));
			break;
		case LastMonth:
			begTime.setDate(begTime.date().addMonths(-1));
			begTime.setDate(QDate(begTime.date().year(), begTime.date().month(), 1));
			begTime.setTime(QTime(0, 0));
			endTime = begTime;
			endTime.setDate(endTime.date().addMonths(1).addDays(-1));
			endTime.setTime(QTime(23, 59));
			break;
		case Custom:
			return;
	}
	dteBegin_->setDateTime(begTime);
	dteEnd_->setDateTime(endTime);
}

#include "hawidget.h"
void DbDialog::tbSearchHost_clicked() {
	HaWidget* widget = dynamic_cast<HaWidget*>(parent());
	Q_ASSERT(widget != nullptr);
	HostDb* hostDb = &widget->hostAnalyzer_.hostDb_;

	QMutexLocker ml(hostDb);
	QSqlQuery query(hostDb->db_);
	QString searchStr = leSearchHost_->text();
	QString queryStr =
		"SELECT"\
		" PRINTF('%012X', host.mac) AS _mac,"\
		" (ip>>24) ||'.'|| ((ip>>16)&255) ||'.'|| ((ip>>8)&255) ||'.'|| (ip&255) as _ip,"\
		" alias, host, vendor "\
		"FROM host";
	if (searchStr != "") {
		queryStr += " WHERE _mac LIKE :search OR _ip LIKE :search OR alias LIKE :search OR host LIKE :search OR vendor LIKE :search";
	}
	if (!query.prepare(queryStr)) {
		QMessageBox::warning(this, "Error", query.lastError().text());
		return;
	}
	if (searchStr != "") {
		searchStr = "%" + searchStr + "%";
		query.bindValue(":search", searchStr);
	}
	if (!query.exec()) {
		QMessageBox::warning(this, "Error", query.lastError().text());
		return;
	}

	if (hostModel_ == nullptr)
		hostModel_ = new HostModel(this, hostDb);
	hostModel_->setQuery(query);
	hostModel_->setHeaderData(ColumnHostMac, Qt::Horizontal, "mac");
	hostModel_->setHeaderData(ColumnHostIp, Qt::Horizontal, "ip");
	hostModel_->setHeaderData(ColumnHostAlias, Qt::Horizontal, "alias");
	hostModel_->setHeaderData(ColumnHostHost, Qt::Horizontal, "host");
	hostModel_->setHeaderData(ColumnHostVendor, Qt::Horizontal, "vendor");

	hostView_->setModel(hostModel_);
	hostView_->resizeColumnsToContents();
	hostView_->update();
}

void DbDialog::tbSearchLog_clicked() {
	HaWidget* widget = dynamic_cast<HaWidget*>(parent());
	Q_ASSERT(widget != nullptr);
	HostDb* hostDb = &widget->hostAnalyzer_.hostDb_;

	QMutexLocker ml(hostDb);
	QSqlQuery query(hostDb->db_);
	QString searchStr = leSearchLog_->text();
	QString queryStr =
		"SELECT"\
		" PRINTF('%012X', log.mac) as _mac,"\
		" (log.ip>>24) ||'.'|| ((log.ip>>16)&255) ||'.'|| ((log.ip>>8)&255) ||'.'|| (log.iP&255) as _ip,"\
		" log.beg_time as beg_time, log.end_time as end_time,"\
		" host.alias as alias, host.host as host, host.vendor as vendor "\
		"FROM log, host "\
		"WHERE (host.mac = log.mac)"\
		" AND ((:begTime <= beg_time AND beg_time <= :endTime) OR (:begTime <= end_time AND end_time <= :endTime) OR (beg_time <= :begTime AND :endTime <= end_time))";
	if (searchStr != "") {
		queryStr += " AND (_mac LIKE :search OR _ip LIKE :search OR alias LIKE :search OR host LIKE :search OR vendor LIKE :search)";
	}
	if (!query.prepare(queryStr)) {
		QMessageBox::warning(this, "Error", query.lastError().text());
		return;
	}
	query.bindValue(":begTime", dteBegin_->dateTime().toSecsSinceEpoch());
	query.bindValue(":endTime", dteEnd_->dateTime().toSecsSinceEpoch());
	if (searchStr != "") {
		searchStr = "%" + searchStr + "%";
		query.bindValue(":search", searchStr);
	}
	if (!query.exec()) {
		QMessageBox::warning(this, "Error", query.lastError().text());
		return;
	}

	if (logModel_ == nullptr)
		logModel_ = new LogModel(this, hostDb);
	logModel_->setQuery(query);
	logModel_->setHeaderData(ColumnLogName, Qt::Horizontal, "name");
	logModel_->setHeaderData(ColumnLogIp, Qt::Horizontal, "ip");
	logModel_->setHeaderData(ColumnLogBegTime, Qt::Horizontal, "beg_time");
	logModel_->setHeaderData(ColumnLogEndTime, Qt::Horizontal, "end_time");

	logView_->setModel(logModel_);
	logView_->hideColumn(ColumnLogAlias);
	logView_->hideColumn(ColumnLogHost);
	logView_->hideColumn(ColumnLogVendor);
	logView_->resizeColumnsToContents();
	logView_->update();
}

void DbDialog::cbPeriod_currentIndexChanged(int index) {
	(void)index;
	setPeriod();
}
