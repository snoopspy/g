#include "bfwidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>

BfWidget::BfWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("Beacon Flood");

	plainTextEdit_ = new GPlainTextEdit(this);
	mainLayout_->addWidget(plainTextEdit_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &BfWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &BfWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &BfWidget::tbOption_clicked);

	beaconFlood_.plainTextEdit_ = plainTextEdit_;

	setControl();
}

BfWidget::~BfWidget() {
	tbStop_->click();
	setControl();
}

void BfWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["bf"] >> beaconFlood_;
}

void BfWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["bf"] << beaconFlood_;
}

void BfWidget::setControl() {
	bool active = beaconFlood_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
	plainTextEdit_->setEnabled(!active);
}

void BfWidget::tbStart_clicked(bool checked) {
	(void)checked;

	if (!beaconFlood_.open()) {
		tbStop_->click();
		QMessageBox::warning(this, "Error", beaconFlood_.err->msg());
		return;
	}

	setControl();
}

void BfWidget::tbStop_clicked(bool checked) {
	(void)checked;

	beaconFlood_.close();
	setControl();
}

#include <GPropDialog>
void BfWidget::tbOption_clicked(bool checked) {
	(void)checked;

	GPropDialog propDialog;
	propDialog.widget_.setObject(&beaconFlood_);

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

void BfWidget::processClosed() {
	if (beaconFlood_.active())
		tbStop_->click();
}
