#include "hawidget.h"

#include <QItemDelegate>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>
#include <GItemDelegate>

HaWidget::HaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("HostAnalyzer");

	treeWidget_ = new GTreeWidget(this);
#ifdef Q_OS_ANDROID
	treeWidget_->header()->setFixedHeight(120);
#endif // Q_OS_ANDROID
	GItemDelegate* itemDelegate = new GItemDelegate(this);
	treeWidget_->setItemDelegate(itemDelegate);
#ifdef Q_OS_ANDROID
	itemDelegate->setHeight(140);
#endif // Q_OS_ANDROID
	treeWidget_->setHeaderLabels(QStringList{"IP", "Name", "Elapsed", ""});
	treeWidget_->setSortingEnabled(true);
	treeWidget_->sortByColumn(-1, Qt::AscendingOrder);
	treeWidget_->setIndentation(0);
	treeWidget_->setEditTriggers(QAbstractItemView::AllEditTriggers);

#ifdef Q_OS_ANDROID
	treeWidget_->setColumnWidth(3, 140);
#else
	treeWidget_->setColumnWidth(3, treeWidget_->header()->sizeHint().height());
#endif // Q_OS_ANDROID

	QHeaderView* hv = treeWidget_->header();
	hv->setSectionResizeMode(2, QHeaderView::Stretch);
	hv->setSectionResizeMode(3, QHeaderView::Fixed);
	hv->setStretchLastSection(false);

	tbDb_ = new QToolButton(this);
	tbDb_->setText("Database");
	tbDb_->setToolTip("Database");
	tbDb_->setIcon(QIcon(":/img/db.png"));
	tbDb_->setAutoRaise(true);
	tbDb_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbDb_);

	tbHost_ = new QToolButton(this);
	tbHost_->setText("Host");
	tbHost_->setToolTip("Host");
	tbHost_->setIcon(QIcon(":/img/edit.png"));
	tbHost_->setAutoRaise(true);
	tbHost_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbHost_);

	tbScreenSaver_ = new QToolButton(this);
	tbScreenSaver_->setText("ScreenSaver");
	tbScreenSaver_->setToolTip("ScreenSaver");
	tbScreenSaver_->setIcon(QIcon(":/img/screensaver.png"));
	tbScreenSaver_->setAutoRaise(true);
	tbScreenSaver_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbScreenSaver_);

	mainLayout_->addWidget(treeWidget_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

	QObject::connect(treeWidget_, &QTreeWidget::itemSelectionChanged, this, &HaWidget::setControl);
	QObject::connect(tbStart_, &QToolButton::clicked, this, &HaWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &HaWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &HaWidget::tbOption_clicked);
	QObject::connect(tbDb_, &QToolButton::clicked, this, &HaWidget::tbDb_clicked);
	QObject::connect(tbHost_, &QToolButton::clicked, this, &HaWidget::tbHost_clicked);
	QObject::connect(tbScreenSaver_, &QToolButton::clicked, this, &HaWidget::tbScreenSaver_clicked);

	hostAnalyzer_.treeWidget_ = treeWidget_;

	QObject::connect(&hostAnalyzer_, &GStateObj::closed, tbStop_, &QToolButton::click);

	setControl();
}

HaWidget::~HaWidget() {
	tbStop_->click();
	setControl();
}

void HaWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["sizes"] >> GJson::columnSizes(treeWidget_);
	jo["ha"] >> hostAnalyzer_;
}

void HaWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["sizes"] << GJson::columnSizes(treeWidget_);
	jo["ha"] << hostAnalyzer_;
}

void HaWidget::setControl() {
	bool active = hostAnalyzer_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
	tbHost_->setEnabled(active && treeWidget_->selectedItems().count() > 0);
	tbScreenSaver_->setEnabled(active);
}

void HaWidget::tbStart_clicked(bool checked) {
	(void)checked;

	treeWidget_->clear();

	if (!hostAnalyzer_.open()) {
		tbStop_->click();
		QMessageBox::warning(this, "Error", hostAnalyzer_.err->msg());
		return;
	}

	setControl();
}

void HaWidget::tbStop_clicked(bool checked) {
	(void)checked;

	hostAnalyzer_.close();
	setControl();
}

#include <GPropDialog>
void HaWidget::tbOption_clicked(bool checked) {
	(void)checked;

	GPropDialog propDialog;
	propDialog.setModal(true);
	propDialog.setWindowTitle("Option");
	propDialog.widget_.setObject(&hostAnalyzer_);

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("propDialog") == jo.end();
	jo["propDialog"] >> propDialog;

#ifdef Q_OS_ANDROID
	propDialog.showMaximized();
#else
	propDialog.show();
#endif
	if (isFirst) {
		int width = propDialog.width();
		propDialog.widget_.treeWidget_->setColumnWidth(0, width / 2);
	}

	propDialog.exec();

	jo["propDialog"] << propDialog;
}

#include "dbdialog.h"
void HaWidget::tbDb_clicked(bool checked) {
	(void)checked;

	GHostDb* hostDb = &hostAnalyzer_.hostDb_;
	bool dbOpened = hostDb->active();
	if (!dbOpened) {
		if (!hostDb->open()) {
			QMessageBox::warning(this, "Error", hostDb->err->msg());
			hostDb->close();
			return;
		}
	}

	DbDialog dbDialog(this, hostDb);
	dbDialog.setModal(true);

	QJsonObject& jo = GJson::instance();
	jo["dbDialog"] >> dbDialog;
	dbDialog.setPeriod();

#ifdef Q_OS_ANDROID
	dbDialog.showMaximized();
#else // Q_OS_ANDROID
	dbDialog.show();
#endif // Q_OS_ANDROID
	dbDialog.exec();

	jo["dbDialog"] << dbDialog;

	// Change default name because alias can be changed
	int count = treeWidget_->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		GTreeWidgetItem *twi = dynamic_cast<GTreeWidgetItem *>(treeWidget_->topLevelItem(i));
		Q_ASSERT(twi != nullptr);
		hostAnalyzer_.updateHost(twi);
	}

	if (!dbOpened)
		hostDb->close();
}

#include "hostdialog.h"
void HaWidget::tbHost_clicked(bool checked) {
	(void)checked;

	if (treeWidget_->selectedItems().count() == 0) return;
	GTreeWidgetItem* twi = PTreeWidgetItem(treeWidget_->selectedItems().at(0));
	GHostMgr::HostValue* hostValue = GHostMgr::PHostValue(twi->property("hostValue").toULongLong());
	Q_ASSERT(hostValue != nullptr);
	GMac mac = hostAnalyzer_.hostDb_.getItem(hostValue)->mac_;
	HostDialog hostDialog(this, mac, &hostAnalyzer_, hostValue);
	hostDialog.setModal(true);

	QJsonObject& jo = GJson::instance();
	jo["hostDialog"] >> hostDialog;

#ifdef Q_OS_ANDROID
	hostDialog.showMaximized();
#else
	hostDialog.show();
#endif
	hostDialog.exec();

	int res = hostDialog.result();
	qDebug() << res;
	if (res == QDialog::Accepted)
		hostAnalyzer_.updateHost(twi);

	jo["hostDialog"] << hostDialog;
}

void HaWidget::tbScreenSaver_clicked(bool checked) {
	(void)checked;
	GScreenSaver* screenSaver = &hostAnalyzer_.screenSaver_;
	if (screenSaver->active())
		screenSaver->close();
	screenSaver->open();
}

void HaWidget::processClosed() {
	if (hostAnalyzer_.active())
		tbStop_->click();
}
