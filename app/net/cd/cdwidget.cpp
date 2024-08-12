#include "cdwidget.h"

#include <QItemDelegate>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>
#include <GItemDelegate>

CdWidget::CdWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("CycleDetect");

	treeWidget_ = new GTreeWidget(this);
	treeWidget_->setHeaderLabels(QStringList{
		"ClientIP",
		"ServerIP",
		"ServerPort",
		"Ttl",
		"Count",
		"FTA",
		"FTAD",
		"LTA",
		"LTAD",
		"TPA",
		"TPAD",
		"TBA",
		"TBADV",
		"RPA",
		"RPAD",
		"RBA",
		"RBAD"
	});
	treeWidget_->setSortingEnabled(true);
	treeWidget_->sortByColumn(-1, Qt::AscendingOrder);
	treeWidget_->setIndentation(0);
	treeWidget_->setEditTriggers(QAbstractItemView::AllEditTriggers);

	QHeaderView* hv = treeWidget_->header();
	hv->setStretchLastSection(true);

	tbEdit_ = new QToolButton(this);
	tbEdit_->setText("Edit");
	tbEdit_->setToolTip("Edit");
	tbEdit_->setIcon(QIcon(":/img/edit.png"));
	tbEdit_->setAutoRaise(true);
	tbEdit_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbEdit_);

	mainLayout_->addWidget(treeWidget_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

	QObject::connect(treeWidget_, &QTreeWidget::itemSelectionChanged, this, &CdWidget::setControl);
	QObject::connect(tbStart_, &QToolButton::clicked, this, &CdWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &CdWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &CdWidget::tbOption_clicked);
	QObject::connect(tbEdit_, &QToolButton::clicked, this, &CdWidget::tbEdit_clicked);

	cycleDetect_.treeWidget_ = treeWidget_;

	QObject::connect(&cycleDetect_, &GStateObj::closed, tbStop_, &QToolButton::click);

	setControl();
}

CdWidget::~CdWidget() {
	tbStop_->click();
	setControl();
}

#include <QCloseEvent>
void CdWidget::closeEvent(QCloseEvent* event) {
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

void CdWidget::setControl() {
	bool active = cycleDetect_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
	tbEdit_->setEnabled(treeWidget_->selectedItems().count() > 0);
}

void CdWidget::tbStart_clicked(bool checked) {
	(void)checked;

	treeWidget_->clear();

	if (!cycleDetect_.open()) {
		tbStop_->click();
		QMessageBox::warning(this, "Error", cycleDetect_.err->msg());
		return;
	}

	setControl();
}

void CdWidget::tbStop_clicked(bool checked) {
	(void)checked;

	cycleDetect_.close();
	setControl();
}

#include <GPropDialog>
void CdWidget::tbOption_clicked(bool checked) {
	(void)checked;

	GPropDialog propDialog;
	propDialog.setModal(true);
	propDialog.setWindowTitle("Option");
	propDialog.widget_.setObject(&cycleDetect_);

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

void CdWidget::tbEdit_clicked(bool checked) {
	(void)checked;
}

void CdWidget::processClosed() {
	if (cycleDetect_.active())
		tbStop_->click();
}

void CdWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["sizes"] >> GJson::columnSizes(treeWidget_);
	jo["cd"] >> cycleDetect_;
}

void CdWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["sizes"] << GJson::columnSizes(treeWidget_);
	jo["cd"] << cycleDetect_;
}
