#include "chwidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>

ChWidget::ChWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("CookieHijack");

	treeWidget_ = new GTreeWidget(this);
	treeWidget_->setColumnCount(CookieHijack::ColumnSignal + 1);
	treeWidget_->setHeaderLabels(QStringList{"Mac", "TY", "CH", "Signal"});
	mainLayout_->addWidget(treeWidget_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &ChWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &ChWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &ChWidget::tbOption_clicked);

	setControl();
}

ChWidget::~ChWidget() {
	tbStop_->click();
	setControl();
}

void ChWidget::setControl() {
	bool active = cookieHijack_.active();
	tbStart_->setEnabled(!active);
	tbStop_->setEnabled(active);
	tbOption_->setEnabled(!active);
}

#include <QCloseEvent>
void ChWidget::closeEvent(QCloseEvent* event) {
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

void ChWidget::tbStart_clicked(bool checked) {
	(void)checked;

	treeWidget_->clear();

	if (!cookieHijack_.open()) {
		tbStop_->click();
		QMessageBox::warning(this, "Error", cookieHijack_.err->msg());
		return;
	}

	setControl();
}

void ChWidget::tbStop_clicked(bool checked) {
	(void)checked;

	cookieHijack_.close();
	setControl();
}

#include <GPropDialog>
void ChWidget::tbOption_clicked(bool checked) {
	(void)checked;

	GPropDialog propDialog;
	propDialog.widget_.setObject(&cookieHijack_);

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

void ChWidget::processProbeDetected(GMac mac, QString type, int channel, int signal) {
	(void)mac;
	(void)type;
	(void)channel;
	(void)signal;
}

void ChWidget::processClosed() {
	if (cookieHijack_.active())
		tbStop_->click();
}

void ChWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["ch"] >> cookieHijack_;
	jo["sizes"] >> GJson::columnSizes(treeWidget_);
}

void ChWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["ch"] << cookieHijack_;
	jo["sizes"] << GJson::columnSizes(treeWidget_);
}
