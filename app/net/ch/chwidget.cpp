#include "chwidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>

ChWidget::ChWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("CookieHijack");

	treeWidget_ = new GTreeWidget(this);
	treeWidget_->setColumnCount(ColumnCookie + 1);
	treeWidget_->setHeaderLabels(QStringList{"Host", "Cookie"});
	mainLayout_->addWidget(treeWidget_);

	tbFirefox_ = new QToolButton(this);
	tbFirefox_->setText("Go");
	tbFirefox_->setToolTip("Go");
	tbFirefox_->setIcon(QIcon(":/img/firefox.png"));
	tbFirefox_->setAutoRaise(true);
	tbFirefox_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbFirefox_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

	QObject::connect(treeWidget_, &QTreeWidget::itemSelectionChanged, this, &ChWidget::setControl);
	QObject::connect(tbStart_, &QToolButton::clicked, this, &ChWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &ChWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &ChWidget::tbOption_clicked);
	QObject::connect(tbFirefox_, &QToolButton::clicked, this, &ChWidget::tbFirefox_clicked);

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
	tbFirefox_->setEnabled(!active && treeWidget_->selectedItems().count() != 0);
}

void ChWidget::addItem(QString host, QString cookie) {
	GTreeWidgetItem* twi = new GTreeWidgetItem(treeWidget_);
	twi->setText(ColumnHost, host);
	twi->setText(ColumnCookie, cookie);
	treeWidget_->addTopLevelItem(twi);
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

	GPropDialog propDialog(this);
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

void ChWidget::tbFirefox_clicked(bool checked) {
	(void)checked;

	if (treeWidget_->selectedItems().count() == 0) return;
	GTreeWidgetItem* twi = PTreeWidgetItem(treeWidget_->selectedItems().at(0));
	QString host = twi->text(ColumnHost);
	if (host.startsWith(cookieHijack_.prefix_ + "."))
		host = host.mid(cookieHijack_.prefix_.size());
	qsizetype i = host.indexOf(":");
	if (i != -1)
		host = host.left(i);
	QString cookie = twi->text(ColumnCookie);

	QString program = QDir::currentPath() + "/ffce";
	QStringList arguments;
	QString firefoxDir = cookieHijack_.firefoxDir_;
	if (firefoxDir == "")
		firefoxDir = QDir::homePath() +"/.mozilla/firefox";
	arguments.push_back(firefoxDir);
	arguments.push_back(host);
	arguments.push_back(cookie);
	QProcess::execute(program, arguments);

	if (host.startsWith("."))
		host = host.mid(1);
	QProcess::startDetached("firefox", QStringList{host});
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
