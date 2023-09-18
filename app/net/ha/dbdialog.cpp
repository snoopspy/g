#include "dbdialog.h"

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
}

DbDialog::~DbDialog() {
}

void DbDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
}

void DbDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
}

#include "hawidget.h"
int DbDialog::exec() {
	HaWidget* widget = dynamic_cast<HaWidget*>(parent());
	Q_ASSERT(widget != nullptr);

	HostDb* hostDb  = &widget->hostAnalyzer_.hostDb_;
	QSqlTableModel* hostModel;
	QSqlTableModel* logModel;
	{
		QMutexLocker ml(hostDb);

		QSqlDatabase db = hostDb->db_;

		hostModel = new HostModel(this, db);
		hostModel->setTable("host");
		hostModel->select();
		hostView_->setModel(hostModel);
		hostView_->resizeColumnsToContents();

		logModel = new LogModel(this, db, &widget->hostAnalyzer_.hostDb_);
		logModel->setTable("log");
		logModel->select();
		logModel->setHeaderData(0, Qt::Horizontal, "Name");
		logView_->setModel(logModel);
		logView_->resizeColumnsToContents();
	}

	int res = QDialog::exec();

	delete hostModel;
	delete logModel;

	return res;
}
