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

	mainLayout_  = new QVBoxLayout(this);
	mainLayout_->setContentsMargins(0, 0, 0, 0);
	tabWidget_ = new QTabWidget(this);

	hostView_ = new QTableView(this);
	logView_ = new QTableView(this);

	tabWidget_->addTab(hostView_, "Host");
	tabWidget_->addTab(logView_, "Log");

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
	QSqlDatabase db = widget->hostAnalyzer_.hostDb_.db_;

	QSqlTableModel* hostModel = new HostModel(this, db);
	hostModel->setTable("host");
	hostModel->select();
	hostView_->setModel(hostModel);
	hostView_->resizeColumnsToContents();

	QSqlTableModel* logModel = new LogModel(this, db, &widget->hostAnalyzer_.hostDb_);
	logModel->setTable("log");
	logModel->select();
	logModel->setHeaderData(0, Qt::Horizontal, "Name");
	logView_->setModel(logModel);
	logView_->resizeColumnsToContents();

	int res = QDialog::exec();

	delete hostModel;
	delete logModel;

	return res;
}
