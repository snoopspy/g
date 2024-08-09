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
	treeWidget_->setHeaderLabels(QStringList{"IP", "Name", "Elapsed", ""});
	treeWidget_->setSortingEnabled(true);
	treeWidget_->sortByColumn(-1, Qt::AscendingOrder);
	treeWidget_->setIndentation(0);
	treeWidget_->setEditTriggers(QAbstractItemView::AllEditTriggers);

#ifdef Q_OS_ANDROID
	treeWidget_->setColumnWidth(HostAnalyzer::ColumnAttack, GItemDelegate::DefaultItemHeight);
#else
	treeWidget_->setColumnWidth(HostAnalyzer::ColumnAttack, treeWidget_->header()->sizeHint().height());
#endif // Q_OS_ANDROID

	QHeaderView* hv = treeWidget_->header();
	hv->setSectionResizeMode(HostAnalyzer::ColumnElapsed, QHeaderView::Stretch);
	hv->setSectionResizeMode(HostAnalyzer::ColumnAttack, QHeaderView::Fixed);
	hv->setStretchLastSection(false);

	tbDb_ = new QToolButton(this);
	tbDb_->setText("Database");
	tbDb_->setToolTip("Database");
	tbDb_->setIcon(QIcon(":/img/db.png"));
	tbDb_->setAutoRaise(true);
	tbDb_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbDb_);

	tbEdit_ = new QToolButton(this);
	tbEdit_->setText("Edit");
	tbEdit_->setToolTip("Edit");
	tbEdit_->setIcon(QIcon(":/img/edit.png"));
	tbEdit_->setAutoRaise(true);
	tbEdit_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbEdit_);

	tbQrCode_ = new QToolButton(this);
	tbQrCode_->setText("QrCode");
	tbQrCode_->setToolTip("QrCode");
	tbQrCode_->setIcon(QIcon(":/img/qrcode.png"));
	tbQrCode_->setAutoRaise(true);
	tbQrCode_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbQrCode_);

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
	QObject::connect(tbEdit_, &QToolButton::clicked, this, &HaWidget::tbEdit_clicked);
	QObject::connect(tbQrCode_, &QToolButton::clicked, this, &HaWidget::tbQrCode_clicked);
	QObject::connect(tbScreenSaver_, &QToolButton::clicked, this, &HaWidget::tbScreenSaver_clicked);

	hostAnalyzer_.treeWidget_ = treeWidget_;

	QObject::connect(&hostAnalyzer_, &GStateObj::closed, tbStop_, &QToolButton::click);

	setControl();
}

HaWidget::~HaWidget() {
	tbStop_->click();
	setControl();
}

#include <QCloseEvent>
void HaWidget::closeEvent(QCloseEvent* event) {
#ifdef Q_OS_ANDROID
	QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm", "Are you sure want to exit?", QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
		event->accept();
	else
		event->ignore();
#else
	event->accept();
#endif // Q_OS_ANDROID
}

void HaWidget::setControl() {
	bool active = hostAnalyzer_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
	tbEdit_->setEnabled(treeWidget_->selectedItems().count() > 0);
	tbQrCode_->setEnabled(active);
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

	if (!dbOpened)
		hostDb->close();
}

#include "hostdialog.h"
void HaWidget::tbEdit_clicked(bool checked) {
	(void)checked;

	if (treeWidget_->selectedItems().count() == 0) return;
	GTreeWidgetItem* twi = PTreeWidgetItem(treeWidget_->selectedItems().at(0));
	GMac mac = twi->property("mac").toString();

	GHostDb* hostDb = &hostAnalyzer_.hostDb_;
	bool dbOpened = hostDb->active();
	if (!dbOpened) {
		if (!hostDb->open()) {
			QMessageBox::warning(this, "Error", hostDb->err->msg());
			hostDb->close();
			return;
		}
	}

	HostDialog hostDialog(this, mac, &hostAnalyzer_);
	hostDialog.setModal(true);

	QJsonObject& jo = GJson::instance();
	jo["hostDialog"] >> hostDialog;

#ifdef Q_OS_ANDROID
	hostDialog.showMaximized();
#else
	hostDialog.show();
#endif
	hostDialog.exec();

	jo["hostDialog"] << hostDialog;

	if (!dbOpened)
		hostDb->close();
}

#include "qrcodedialog.h"
void HaWidget::tbQrCode_clicked(bool checked) {
	(void)checked;

	QrCodeDialog qrCodeDialog(this);
	qrCodeDialog.setModal(true);

	QJsonObject& jo = GJson::instance();
	jo["qrCodeDialog"] >> qrCodeDialog;

#ifdef Q_OS_ANDROID
	qrCodeDialog.showMaximized();
#else
	qrCodeDialog.show();
#endif
	qrCodeDialog.exec();

	jo["qrCodeDialog"] << qrCodeDialog;
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
