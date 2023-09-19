#include "dbdialog.h"

#include <QMessageBox>
#include <QSqlTableModel>
#include <GMac>
#include <GIp>
#include <GJson>

struct HostModel : QSqlTableModel {
	explicit HostModel(QObject *parent, QSqlDatabase db) : QSqlTableModel(parent, db) {}

	QVariant data(const QModelIndex &index, int role) const override {
		QVariant res = QSqlTableModel::data(index, role);
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
				case 0: { // mac
					GMac mac = GMac(res.toULongLong());
					return QString(mac);
				}
				case 1: { // ip
					GIp ip = GIp(res.toUInt());
					return QString(ip);
				}
			}
		}
		return res;
	}

	Qt::ItemFlags flags(const QModelIndex &index) const override {
		Qt::ItemFlags res = QSqlTableModel::flags(index);
		if (index.column() != 2) // alias
			res &= ~Qt::ItemIsEditable;
		return res;
	}
};

#include "hostdb.h"
struct LogModel : QSqlTableModel {
	HostDb* hostDb_;
	explicit LogModel(QObject *parent, QSqlDatabase db, HostDb* hostDb) : QSqlTableModel(parent, db), hostDb_(hostDb) {}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		QVariant res = QSqlTableModel::data(index, role);
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
				case 0: { // mac
					GMac mac = GMac(res.toULongLong());
					return hostDb_->getDefaultName(mac, nullptr);
				}
				case 1: { // ip
					GIp ip = GIp(res.toUInt());
					return QString(ip);
				}
				case 2: // beg_time
				case 3: { // end_time
					QDateTime dt = QDateTime::fromSecsSinceEpoch(res.toULongLong());
					return dt.toString("MMdd hh:mm");
				}
			}
		}
		return res;
	}

	Qt::ItemFlags flags(const QModelIndex &index) const override {
		Qt::ItemFlags res = QSqlTableModel::flags(index);
		if (index.column() != 2) // alias
			res &= ~Qt::ItemIsEditable;
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
				cbPeriod_->addItems(QStringList{"1 Hour", "2 Hour", "3 Hour", "Today", "Yesterday", "This Week", "Last Week", "This Month", "Last Month", "Custom"});
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
}

DbDialog::~DbDialog() {
}

void DbDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	tabWidget_->setCurrentIndex(jo["tabIndex"].toInt(0));
	dteBegin_->setDateTime(QDateTime::fromString(jo["begTime"].toString(), "yy/MM/dd hh:mm"));
	dteEnd_->setDateTime(QDateTime::fromString(jo["endTime"].toString(), "yy/MM/dd hh:mm"));
	cbPeriod_->setCurrentIndex(jo["searchPeriod"].toInt(0));
}

void DbDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["tabIndex"] = tabWidget_->currentIndex();
	jo["begTime"] = dteBegin_->dateTime().toString("yy/MM/dd hh:mm");
	jo["endTime"] = dteEnd_->dateTime().toString("yy/MM/dd hh:mm");
	jo["searchPeriod"] = cbPeriod_->currentIndex();
}

void DbDialog::setPeriod() {
	SearchPeriod period = SearchPeriod(cbPeriod_->currentIndex());
	QDateTime begTime = QDateTime::currentDateTime();
	QDateTime endTime = begTime;
	switch (period) {
		case Hour1:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600); // 1 Hour
			break;
		case Hour2:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600 * 2); // 2 Hour
			break;
		case Hour3:
			begTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600 * 3); // 3 Hour
			break;
		case Today:
			begTime.setTime(QTime(0, 0));
			break;
		case Yesterday: {
			begTime.setDate(begTime.addDays(-1).date());
			begTime.setTime(QTime(0, 0));
			endTime.setDate(begTime.addDays(-1).date());
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
	QString queryStr = "SELECT PRINTF('%012X', mac) AS _mac, (ip>>24) ||'.'|| ((ip>>16)&255) ||'.'|| ((ip>>8)&255) ||'.'|| (ip&255) as _ip, alias, host, vendor FROM host";
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
		hostModel_ = new QSqlQueryModel(this);
	hostModel_->setQuery(query);
	hostModel_->setQuery(query);
	hostView_->setModel(hostModel_);
	hostView_->resizeColumnsToContents();
	hostView_->update();
}

void DbDialog::tbSearchLog_clicked() {
	qDebug() << "";
}

void DbDialog::cbPeriod_currentIndexChanged(int index) {
	(void)index;
	setPeriod();
}
