#include "wawidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QScrollBar>

#include <GJson>

WaWidget::WaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("WiFiAnalyzer");

	plainTextEdit_ = new GPlainTextEdit(this);
	plainTextEdit_->setReadOnly(true);
	plainTextEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	mainLayout_->addWidget(plainTextEdit_);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &WaWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &WaWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &WaWidget::tbOption_clicked);

	QObject::connect(&wifiJammer_, &WifiAnalyzer::attacked, this, &WaWidget::processAttacked);
	QObject::connect(&wifiJammer_.monitorDevice_, &GMonitorDevice::closed, this, &WaWidget::processClosed);

	setControl();
}

WaWidget::~WaWidget() {
	tbStop_->click();
	setControl();
}

void WaWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["pa"] >> wifiJammer_;
}

void WaWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["pa"] << wifiJammer_;
}

void WaWidget::setControl() {
	bool active = wifiJammer_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
}

void WaWidget::tbStart_clicked(bool checked) {
	(void)checked;

	plainTextEdit_->clear();

	if (!wifiJammer_.open()) {
		QMessageBox::warning(this, "Error", wifiJammer_.err->msg());
	}
	setControl();
}

void WaWidget::tbStop_clicked(bool checked) {
	(void)checked;

	wifiJammer_.close();
	setControl();
}

#include <GPropDialog>
void WaWidget::tbOption_clicked(bool checked) {
	(void)checked;

	GPropDialog propDialog;
	propDialog.widget_.setObject(&wifiJammer_);

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

void WaWidget::processAttacked(GMac mac, QString ssid) {
	QString msg = QString("%1 %2").arg(QString(mac)).arg(ssid);
	qDebug() << msg;
	plainTextEdit_->insertPlainText(msg + "\n");
	plainTextEdit_->verticalScrollBar()->setValue(plainTextEdit_->verticalScrollBar()->maximum());
}

void WaWidget::processClosed() {
	tbStop_->click();
}
