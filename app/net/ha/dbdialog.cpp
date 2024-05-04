#include "dbdialog.h"

#include <QMessageBox>
#include <QSqlTableModel>
#include <GMac>
#include <GIp>
#include <GJson>

struct HostModel : QSqlQueryModel {
	GHostDb* hostDb_;
	explicit HostModel(QObject *parent, GHostDb* hostDb) : QSqlQueryModel(parent), hostDb_(hostDb) {}

	Qt::ItemFlags flags(const QModelIndex &index) const override {
		Qt::ItemFlags res = QSqlQueryModel::flags(index);
		int column = index.column();
		switch (column) {
			case DbDialog::ColumnHostAlias :
			case DbDialog::ColumnHostMode :
				res |= Qt::ItemIsEditable;
				break;
			default:
				break;
		}
		return res;
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		QVariant res = QSqlQueryModel::data(index, role);
		if (role == Qt::DisplayRole) {
			if (index.column() == DbDialog::ColumnHostMac) {
				GMac mac = res.toString();
				return QString(mac);
			}
		}
		return res;
	}

	bool setData(const QModelIndex &index, const QVariant &value, int role) override {
		if (role != Qt::EditRole)
			QSqlQueryModel::setData(index, value, role);

		QModelIndex macIndex = index.siblingAtColumn(DbDialog::ColumnHostMac);
		GMac mac = macIndex.data().toString();

		GHostDb::Item dbItem;
		if (!hostDb_->selectHost(mac, &dbItem)) {
			qWarning() << QString("hostDb_.selectHost(%1) return false").arg(QString(mac));
			return false;
		}

		int column = index.column();
		switch (column) {
			case DbDialog::ColumnHostAlias :
				dbItem.alias_ = value.toString();
				break;
			case DbDialog::ColumnHostMode :
				dbItem.mode_ = GHostDb::Mode(value.toInt());
				break;
		}
		if (!hostDb_->updateHost(mac, &dbItem)) {
			qWarning() << QString("hostDb_.updateHost(%1) return false").arg(QString(mac));
			return false;
		}

		HostAnalyzer* hostAnalyzer = dynamic_cast<HostAnalyzer*>(hostDb_->parent());
		Q_ASSERT(hostAnalyzer != nullptr);
		GHostMgr::HostMap* hostMap = &hostAnalyzer->hostMgr_.hostMap_;
		Q_ASSERT(hostMap != nullptr);
		QMutexLocker ml(hostMap);
		GHostMgr::HostMap::iterator it = hostMap->find(mac);
		if (it != hostMap->end()) {
			GHostMgr::HostValue* hostValue = it.value();
			Q_ASSERT(hostValue != nullptr);
			HostAnalyzer::Item* haItem = hostAnalyzer->getItem(hostValue);
			haItem->state_ = HostAnalyzer::Item::Changed;
			if (dbItem.mode_ == GHostDb::Default)
				haItem->blockTime_ = hostValue->firstTime_ + hostAnalyzer->admitTimeoutSec_;
			else
				haItem->blockTime_ = 0;
			hostAnalyzer->checkBlockTime(hostValue);
		}

		// Refresh updated data
		DbDialog* dbDialog = dynamic_cast<DbDialog*>(parent());
		Q_ASSERT(dbDialog != nullptr);
		dbDialog->tbSearchHost_->click();
		return true;
	}
};

struct LogModel : QSqlQueryModel {
	GHostDb* hostDb_;
	explicit LogModel(QObject *parent, GHostDb* hostDb) : QSqlQueryModel(parent), hostDb_(hostDb) {}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		QVariant res = QSqlQueryModel::data(index, role);
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
				case DbDialog::ColumnLogName: {
					GMac mac = res.toString();
					GHostDb::Item dbItem;
					hostDb_->selectHost(mac, &dbItem);
					return dbItem.getDefaultName();
				}
				case DbDialog::ColumnLogIp: break;
				case DbDialog::ColumnLogSttTime:
				case DbDialog::ColumnLogEndTime: {
					QDateTime dt = QDateTime::fromSecsSinceEpoch(res.toULongLong());
					return dt.toString("MMdd hh:mm");
				}
			}
		}
		return res;
	}
};

DbDialog::DbDialog(QWidget* parent, GHostDb* hostDb) : QDialog(parent), hostDb_(hostDb) {
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
				tbSearchHost_->setText("Search Host");
				tbSearchHost_->setToolTip("Search Host");
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
			dteStart_ = new QDateTimeEdit(this);
			dteStart_->setDisplayFormat("yy/MM/dd hh:mm");
			logHLayout_->addWidget(dteStart_);
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
				tbSearchLog_->setText("Search Log");
				tbSearchLog_->setToolTip("Search Log");
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

void DbDialog::setPeriod() {
	SearchPeriod period = SearchPeriod(cbPeriod_->currentIndex());
	QDateTime sttTime = QDateTime::currentDateTime();
	QDateTime endTime = sttTime;
	switch (period) {
		case Min10:
			sttTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 600); // 10 minutes
			break;
		case Min20:
			sttTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 1200); // 20 minutes
			break;
		case Min30:
			sttTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 1800); // 30 minutes
			break;
		case Hour1:
			sttTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600); // 1 Hour
			break;
		case Hour2:
			sttTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600 * 2); // 2 Hours
			break;
		case Hour3:
			sttTime = QDateTime::fromSecsSinceEpoch(endTime.toSecsSinceEpoch() - 3600 * 3); // 3 Hours
			break;
		case Today:
			sttTime.setTime(QTime(0, 0));
			break;
		case Yesterday: {
			sttTime.setDate(sttTime.addDays(-1).date());
			sttTime.setTime(QTime(0, 0));
			endTime.setDate(endTime.addDays(-1).date());
			endTime.setTime(QTime(23, 59));
			break;
		}
		case ThisWeek:
			while(sttTime.date().dayOfWeek() != 7) { // Sunday
				sttTime.setDate(sttTime.addDays(-1).date());
			}
			sttTime.setTime(QTime(0, 0));
			break;
		case LastWeek:
			sttTime.setDate(sttTime.addDays(-7).date());
			while(sttTime.date().dayOfWeek() != 7) { // Sunday
				sttTime.setDate(sttTime.addDays(-1).date());
			}
			sttTime.setTime(QTime(0, 0));
			endTime.setDate(endTime.addDays(-7).date());
			while(endTime.date().dayOfWeek() != 6) { // Saturday
				endTime.setDate(endTime.addDays(1).date());
			}
			endTime.setTime(QTime(23, 59));
			break;
		case ThisMonth:
			sttTime.setDate(QDate(sttTime.date().year(), sttTime.date().month(), 1));
			sttTime.setTime(QTime(0, 0));
			break;
		case LastMonth:
			sttTime.setDate(sttTime.date().addMonths(-1));
			sttTime.setDate(QDate(sttTime.date().year(), sttTime.date().month(), 1));
			sttTime.setTime(QTime(0, 0));
			endTime = sttTime;
			endTime.setDate(endTime.date().addMonths(1).addDays(-1));
			endTime.setTime(QTime(23, 59));
			break;
		case Custom:
			return;
	}
	dteStart_->setDateTime(sttTime);
	dteEnd_->setDateTime(endTime);
}

void DbDialog::tbSearchHost_clicked() {
	QMutexLocker ml(hostDb_);
	QSqlQuery query(hostDb_->db_);
	QString searchStr = leSearchHost_->text();
	QString queryStr =
		"SELECT"\
		" PRINTF('%012X', host.mac) AS _mac,"\
		" (ip>>24) ||'.'|| ((ip>>16)&255) ||'.'|| ((ip>>8)&255) ||'.'|| (ip&255) as _ip,"\
		" alias, host, vendor, mode "\
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
		hostModel_ = new HostModel(this, hostDb_);
	hostModel_->setQuery(std::move(query));
	hostModel_->setHeaderData(ColumnHostMac, Qt::Horizontal, "Mac");
	hostModel_->setHeaderData(ColumnHostIp, Qt::Horizontal, "IP");
	hostModel_->setHeaderData(ColumnHostAlias, Qt::Horizontal, "Alias");
	hostModel_->setHeaderData(ColumnHostHost, Qt::Horizontal, "Host");
	hostModel_->setHeaderData(ColumnHostVendor, Qt::Horizontal, "Vendor");
	hostModel_->setHeaderData(ColumnHostMode, Qt::Horizontal, "Mode");

	hostView_->setModel(hostModel_);
	hostView_->resizeColumnsToContents();
	hostView_->update();
}

void DbDialog::tbSearchLog_clicked() {
	QMutexLocker ml(hostDb_);
	QSqlQuery query(hostDb_->db_);
	QString searchStr = leSearchLog_->text();
	QString queryStr =
		"SELECT"\
		" PRINTF('%012X', log.mac) as _mac,"\
		" (log.ip>>24) ||'.'|| ((log.ip>>16)&255) ||'.'|| ((log.ip>>8)&255) ||'.'|| (log.iP&255) as _ip,"\
		" log.stt_time as stt_time, log.end_time as end_time,"\
		" host.alias as alias, host.host as host, host.vendor as vendor "\
		"FROM log, host "\
		"WHERE (host.mac = log.mac)"\
		" AND ((:sttTime <= stt_time AND stt_time <= :endTime) OR (:sttTime <= end_time AND end_time <= :endTime) OR (stt_time <= :sttTime AND :endTime <= end_time))";
	if (searchStr != "") {
		queryStr += " AND (_mac LIKE :search OR _ip LIKE :search OR alias LIKE :search OR host LIKE :search OR vendor LIKE :search)";
	}
	if (!query.prepare(queryStr)) {
		QMessageBox::warning(this, "Error", query.lastError().text());
		return;
	}
	query.bindValue(":sttTime", dteStart_->dateTime().toSecsSinceEpoch());
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
		logModel_ = new LogModel(this, hostDb_);
	logModel_->setQuery(std::move(query));
	logModel_->setHeaderData(ColumnLogName, Qt::Horizontal, "Name");
	logModel_->setHeaderData(ColumnLogIp, Qt::Horizontal, "IP");
	logModel_->setHeaderData(ColumnLogSttTime, Qt::Horizontal, "Start");
	logModel_->setHeaderData(ColumnLogEndTime, Qt::Horizontal, "End");

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

void DbDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	tabWidget_->setCurrentIndex(jo["tabIndex"].toInt(0));
	leSearchHost_->setText(jo["searchHost"].toString());
	dteStart_->setDateTime(QDateTime::fromString(jo["sttTime"].toString(), "yy/MM/dd hh:mm"));
	dteEnd_->setDateTime(QDateTime::fromString(jo["endTime"].toString(), "yy/MM/dd hh:mm"));
	leSearchLog_->setText(jo["searchLog"].toString());
	cbPeriod_->setCurrentIndex(jo["searchPeriod"].toInt(int(Today)));
}

void DbDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["tabIndex"] = tabWidget_->currentIndex();
	jo["searchHost"] = leSearchHost_->text();
	jo["sttTime"] = dteStart_->dateTime().toString("yy/MM/dd hh:mm");
	jo["endTime"] = dteEnd_->dateTime().toString("yy/MM/dd hh:mm");
	jo["searchLog"] = leSearchLog_->text();
	jo["searchPeriod"] = cbPeriod_->currentIndex();
}
