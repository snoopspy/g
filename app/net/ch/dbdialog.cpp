#include "dbdialog.h"

#include <QItemSelectionModel>
#include <QMessageBox>
#include <QModelIndexList>
#include <QSqlTableModel>
#include <GMac>
#include <GIp>
#include <GJson>

struct CookieModel : QSqlQueryModel {
	CookieModel(QObject *parent) : QSqlQueryModel(parent) {}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		QVariant res = QSqlQueryModel::data(index, role);
		if (role == Qt::DisplayRole) {
			switch (index.column()) {
				case DbDialog::ColumnCreated: {
					QDateTime dt = QDateTime::fromSecsSinceEpoch(res.toULongLong());
					return dt.toString("MMdd hh:mm");
				}
			}
		}
		return res;
	}
};

DbDialog::DbDialog(QWidget* parent, GCookieHijack* cookieHijack) : QDialog(parent), cookieHijack_(cookieHijack) {
	resize(QSize(640, 480));
	setWindowTitle("Database");

	mainLayout_ = new QVBoxLayout;
	mainLayout_->setContentsMargins(0, 0, 0, 0);
	mainLayout_->setSpacing(0);

	splitter_ = new GSplitter(Qt::Vertical, this);
	mainLayout_->addWidget(splitter_);
	this->setLayout(mainLayout_);

	tabWidget_ = new QTabWidget(this);
	//
	// Cookie
	//
	cookieWidget_ = new QWidget(this);
		cookieVLayout_ = new QVBoxLayout;
		cookieVLayout_->setContentsMargins(0, 0, 0, 0);
		cookieVLayout_->setSpacing(0);
		cookieWidget_->setLayout(cookieVLayout_);
			cookieHLayout_ = new QHBoxLayout;
			cookieHLayout_->setContentsMargins(0, 0, 0, 0);
			cookieVLayout_->setSpacing(0);
			dteStart_ = new QDateTimeEdit(this);
			dteStart_->setDisplayFormat("yy/MM/dd hh:mm");
			cookieHLayout_->addWidget(dteStart_);
				dteEnd_ = new QDateTimeEdit(this);
				dteEnd_->setDisplayFormat("yy/MM/dd hh:mm");
				cookieHLayout_->addWidget(dteEnd_);
				cbPeriod_ = new QComboBox(this);
				cbPeriod_->addItems(QStringList{"10 Minutes", "20 Minutes", "30 Minutes", "1 Hour", "2 Hours", "3 Hours", "Today", "Yesterday", "This Week", "Last Week", "This Month", "Last Month", "Custom"});
				cbPeriod_->setCurrentIndex(int(Today));
				cookieHLayout_->addWidget(cbPeriod_);
				leSearchCookie_ = new QLineEdit(this);
				cookieHLayout_->addWidget(leSearchCookie_);
				tbSearchCookie_ = new QToolButton(this);
				tbSearchCookie_->setText("Search Cookie");
				tbSearchCookie_->setToolTip("Search Cookie");
				tbSearchCookie_->setIcon(QIcon(":/img/search.png"));
				tbSearchCookie_->setAutoRaise(true);
				cookieHLayout_->addWidget(tbSearchCookie_);
				tbFirefox_ = new QToolButton(this);
				tbFirefox_->setText("Firefox");
				tbFirefox_->setToolTip("Firefox");
				tbFirefox_->setIcon(QIcon(":/img/firefox.png"));
				tbFirefox_->setAutoRaise(true);
				cookieHLayout_->addWidget(tbFirefox_);
			cookieView_ = new GTableView(this);
			cookieView_->verticalHeader()->hide();
		cookieVLayout_->addLayout(cookieHLayout_);
		cookieVLayout_->addWidget(cookieView_);

	tabWidget_->addTab(cookieWidget_, "Cookie");
	splitter_->addWidget(tabWidget_);

	plainTextEdit_= new QPlainTextEdit(this);
	splitter_->addWidget(plainTextEdit_);

	QObject::connect(tbSearchCookie_, &QToolButton::clicked, this, &DbDialog::tbSearchLog_clicked);
	QObject::connect(cbPeriod_, SIGNAL(currentIndexChanged(int)), this, SLOT(cbPeriod_currentIndexChanged(int)));
	QObject::connect(leSearchCookie_, &QLineEdit::returnPressed, this, &DbDialog::tbSearchLog_clicked);
	QObject::connect(tbFirefox_, &QToolButton::clicked, this, &DbDialog::tbFirefox_clicked);

	setControl();
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

void DbDialog::setControl() {
	tbFirefox_->setEnabled(false);
	QItemSelectionModel* model = cookieView_->selectionModel();
	if (model == nullptr)
		return;
	if (model->selectedIndexes().count() == 0)
		return;
	const QModelIndex& modelIndex = model->selectedIndexes().at(0);
	QString cookie = modelIndex.siblingAtColumn(ColumnCookie).data().toString();
	plainTextEdit_->setPlainText(cookie);
	tbFirefox_->setEnabled(true);
}

void DbDialog::tbSearchLog_clicked() {
	QMutexLocker ml(cookieHijack_);
	QSqlQuery query(cookieHijack_->db_);
	QString searchStr = leSearchCookie_->text();
	QString queryStr =
		"SELECT"\
		" created,"\
		" PRINTF('%012X', mac) as _mac,"\
		" (ip>>24) ||'.'|| ((ip>>16)&255) ||'.'|| ((ip>>8)&255) ||'.'|| (iP&255) as _ip,"\
		" host,"\
		" cookie "\
		"FROM cookie "\
		" WHERE (:sttTime <= created AND :endTime >= created)";
	if (searchStr != "") {
		queryStr += " AND (_ip LIKE :search OR host LIKE :search OR cookie LIKE :search)";
	}
	qDebug() << queryStr; // gilgil temp 2024.05.04
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

	if (cookieModel_ == nullptr)
		cookieModel_ = new CookieModel(this);
	cookieModel_->setQuery(std::move(query));
	cookieModel_->setHeaderData(ColumnCreated, Qt::Horizontal, "Created");
	cookieModel_->setHeaderData(ColumnMac, Qt::Horizontal, "Mac");
	cookieModel_->setHeaderData(ColumnIp, Qt::Horizontal, "IP");
	cookieModel_->setHeaderData(ColumnHost, Qt::Horizontal, "Host");
	cookieModel_->setHeaderData(ColumnCookie, Qt::Horizontal, "Cookie");

	cookieView_->setModel(cookieModel_);
	cookieView_->hideColumn(ColumnMac);
	cookieView_->resizeColumnsToContents();
	cookieView_->update();

	QItemSelectionModel* model = cookieView_->selectionModel();
	QObject::connect(model, &QItemSelectionModel::selectionChanged, this, &DbDialog::doSelectionChanged);

	setControl();
}

void DbDialog::cbPeriod_currentIndexChanged(int index) {
	(void)index;
	setPeriod();
}

void DbDialog::doSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
	(void)selected;
	(void)deselected;
	qDebug() << selected.data();
	qDebug() << deselected.data();
	setControl();
}

#include "chwidget.h"
void DbDialog::tbFirefox_clicked() {
	QItemSelectionModel* model = cookieView_->selectionModel();
	if (model == nullptr)
		return;
	if (model->selectedIndexes().count() == 0)
		return;
	const QModelIndex& modelIndex = model->selectedIndexes().at(0);
	QString host = modelIndex.siblingAtColumn(ColumnHost).data().toString();
	QString cookie = modelIndex.siblingAtColumn(ColumnCookie).data().toString();

	ChWidget* chWidget = dynamic_cast<ChWidget*>(parent());
	Q_ASSERT(chWidget != nullptr);
	chWidget->launchFirefox(host, cookie);
}

void DbDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	dteStart_->setDateTime(QDateTime::fromString(jo["sttTime"].toString(), "yy/MM/dd hh:mm"));
	dteEnd_->setDateTime(QDateTime::fromString(jo["endTime"].toString(), "yy/MM/dd hh:mm"));
	leSearchCookie_->setText(jo["searchLog"].toString());
	cbPeriod_->setCurrentIndex(jo["searchPeriod"].toInt(int(Today)));
	jo["splitter"] >> GJson::splitterSizes(splitter_);
}

void DbDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["sttTime"] = dteStart_->dateTime().toString("yy/MM/dd hh:mm");
	jo["endTime"] = dteEnd_->dateTime().toString("yy/MM/dd hh:mm");
	jo["searchLog"] = leSearchCookie_->text();
	jo["searchPeriod"] = cbPeriod_->currentIndex();
	jo["splitter"] << GJson::splitterSizes(splitter_);
}
