#include "hawidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>

HaWidget::HaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("Host Analyzer");

	treeWidget_ = new GTreeWidget(this);

	treeWidget_->setSortingEnabled(true);
	treeWidget_->setHeaderLabels(QStringList{"IP", "Mac", "HostName"});

	mainLayout_->addWidget(treeWidget_);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &HaWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &HaWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &HaWidget::tbOption_clicked);

	hostAnalyzer_.treeWidget_ = treeWidget_;

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
	propDialog.widget_.setObject(&hostAnalyzer_);

	QJsonObject& jo = GJson::instance();
	jo["propDialog"] >> propDialog;

#ifndef Q_OS_ANDROID
	propDialog.show();
#else
	propDialog.showMaximized();
#endif
	propDialog.exec();

	jo["propDialog"] << propDialog;
}

void HaWidget::processClosed() {
	if (hostAnalyzer_.active())
		tbStop_->click();
}
