#include "wjwidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QScrollBar>

#include <GJson>

WjWidget::WjWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("ProbeAnalyzer");

	plainTextEdit_ = new GPlainTextEdit(this);
	plainTextEdit_->setReadOnly(true);
	plainTextEdit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	mainLayout_->addWidget(plainTextEdit_);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &WjWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &WjWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &WjWidget::tbOption_clicked);

	QObject::connect(&wifiJammer_, &WifiJammer::attacked, this, &WjWidget::processAttacked);
	QObject::connect(&wifiJammer_.monitorDevice_, &GMonitorDevice::closed, this, &WjWidget::processClosed);

	setControl();
}

WjWidget::~WjWidget() {
	tbStop_->click();
	setControl();
}

void WjWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["pa"] >> wifiJammer_;
}

void WjWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["pa"] << wifiJammer_;
}

void WjWidget::setControl() {
	bool active = wifiJammer_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
}

void WjWidget::tbStart_clicked(bool checked) {
	(void)checked;

	plainTextEdit_->clear();

	if (!wifiJammer_.open()) {
		QMessageBox::warning(this, "Error", wifiJammer_.err->msg());
	}
	setControl();
}

void WjWidget::tbStop_clicked(bool checked) {
	(void)checked;

	wifiJammer_.close();
	setControl();
}

#include <GPropDialog>
void WjWidget::tbOption_clicked(bool checked) {
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

void WjWidget::processAttacked(GMac mac, QString ssid) {
	QString msg = QString("%1 %2").arg(QString(mac)).arg(ssid);
	qDebug() << msg;
	plainTextEdit_->insertPlainText(msg + "\n");
	plainTextEdit_->verticalScrollBar()->setValue(plainTextEdit_->verticalScrollBar()->maximum());
}

void WjWidget::processClosed() {
	tbStop_->click();
}
