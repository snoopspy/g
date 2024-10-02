#include "cmwidget.h"

#include <QItemDelegate>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>
#include <GItemDelegate>

CmWidget::CmWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("CertManager");

	treeWidget_ = new GTreeWidget(this);
	treeWidget_->setHeaderLabels(QStringList{"Server"});
	//treeWidget_->setSortingEnabled(true);
	//treeWidget_->sortByColumn(-1, Qt::AscendingOrder);
	//treeWidget_->setIndentation(0);
	//treeWidget_->setEditTriggers(QAbstractItemView::AllEditTriggers);

	QHeaderView* hv = treeWidget_->header();
	hv->setSectionResizeMode(0, QHeaderView::Stretch);
	//hv->setStretchLastSection(false);

	mainLayout_->addWidget(treeWidget_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

	QObject::connect(treeWidget_, &QTreeWidget::itemSelectionChanged, this, &CmWidget::setControl);
	QObject::connect(tbStart_, &QToolButton::clicked, this, &CmWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &CmWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &CmWidget::tbOption_clicked);

	certManager_.treeWidget_ = treeWidget_;

	QObject::connect(&certManager_, &GStateObj::closed, tbStop_, &QToolButton::click);

	setControl();
}

CmWidget::~CmWidget() {
	tbStop_->click();
	setControl();
}

#include <QCloseEvent>
void CmWidget::closeEvent(QCloseEvent* event) {
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

void CmWidget::setControl() {
	bool active = certManager_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
}

void CmWidget::tbStart_clicked(bool checked) {
	(void)checked;

	treeWidget_->clear();

	if (!certManager_.open()) {
		tbStop_->click();
		QMessageBox::warning(this, "Error", certManager_.err->msg());
		return;
	}

	setControl();
}

void CmWidget::tbStop_clicked(bool checked) {
	(void)checked;

	certManager_.close();
	setControl();
}

#include <GPropDialog>
void CmWidget::tbOption_clicked(bool checked) {
	(void)checked;

	GPropDialog propDialog;
	propDialog.setModal(true);
	propDialog.setWindowTitle("Option");
	propDialog.widget_.setObject(&certManager_);

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

void CmWidget::processClosed() {
	if (certManager_.active())
		tbStop_->click();
}

void CmWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["sizes"] >> GJson::columnSizes(treeWidget_);
	jo["ha"] >> certManager_;
}

void CmWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["sizes"] << GJson::columnSizes(treeWidget_);
	jo["ha"] << certManager_;
}
