#include "chwidget.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GApp>
#include <GJson>

ChWidget::ChWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("CookieHijack");

	splitter_ = new GSplitter(Qt::Vertical, this);
	mainLayout_->addWidget(splitter_);

	treeWidget_ = new GTreeWidget(this);
	treeWidget_->setColumnCount(ColumnCookie + 1);
	treeWidget_->setHeaderLabels(QStringList{"Host", "Cookie"});
	splitter_->addWidget(treeWidget_);

	plainTextEdit_ = new QPlainTextEdit(this);
	splitter_->addWidget(plainTextEdit_);

	tbDb_ = new QToolButton(this);
	tbDb_->setText("Database");
	tbDb_->setToolTip("Database");
	tbDb_->setIcon(QIcon(":/img/db.png"));
	tbDb_->setAutoRaise(true);
	tbDb_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbDb_);

	tbFirefox_ = new QToolButton(this);
	tbFirefox_->setText("Go");
	tbFirefox_->setToolTip("Go");
	tbFirefox_->setIcon(QIcon(":/img/firefox.png"));
	tbFirefox_->setAutoRaise(true);
	tbFirefox_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbFirefox_);

	tbScreenSaver_ = new QToolButton(this);
	tbScreenSaver_->setText("ScreenSaver");
	tbScreenSaver_->setToolTip("ScreenSaver");
	tbScreenSaver_->setIcon(QIcon(":/img/screensaver.png"));
	tbScreenSaver_->setAutoRaise(true);
	tbScreenSaver_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbScreenSaver_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &ChWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &ChWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &ChWidget::tbOption_clicked);
	QObject::connect(tbDb_, &QToolButton::clicked, this, &ChWidget::tbDb_clicked);
	QObject::connect(tbFirefox_, &QToolButton::clicked, this, &ChWidget::tbFirefox_clicked);
	QObject::connect(tbScreenSaver_, &QToolButton::clicked, this, &ChWidget::tbScreenSaver_clicked);
	QObject::connect(treeWidget_, &QTreeWidget::itemSelectionChanged, this, &ChWidget::treeWidget_itemSelectionChanged);

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
	tbDb_->setEnabled(!active);
	tbFirefox_->setEnabled(!active && treeWidget_->selectedItems().count() != 0);
	tbScreenSaver_->setEnabled(active);
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

void ChWidget::launchFirefox(QString host, QString cookie) {
	//
	// Kill Firefox
	//
	{
#ifdef Q_OS_WIN
		// taskkill /IM firefox.exe /F
		QString  program = "taskkill";
		QStringList arguments;
		arguments.append("/IM");
		arguments.append("firefox.exe");
		arguments.append("/F");
#else // Q_OS_WIN
		QString program = "su";
		QStringList arguments;
		arguments.append("-c");
#ifdef Q_OS_ANDROID
		arguments.append("pkill org.mozilla.firefox");
#else // Q_OS_ANDROID
		arguments.append("pkill firefox");
#endif // Q_OS_ANDROID
#endif // Q_OS_WIN
		QProcess::execute(program, arguments);
		QThread::sleep(1); // waiting firefox process killed
	}

	//
	// Set Cookie using ffce
	//
	if (host.startsWith(cookieHijack_.prefix_ + "."))
		host = host.mid(cookieHijack_.prefix_.size());
	qsizetype i = host.indexOf(":");
	if (i != -1)
		host = host.left(i);

#ifdef Q_OS_WIN
	QString program = "ffce.exe";
#else // Q_OS_WIN
	QString program = "ffce";
#endif // Q_OS_WIN
	QStringList arguments;
	arguments.push_back(cookieHijack_.firefoxDir_);
	arguments.push_back(host);
	arguments.push_back("'" + cookie + "'");
	qDebug() << program << arguments; // gilgil temp 2024.04.15
	if (GApp::prepareProcess(program, arguments))
		QProcess::execute(program, arguments);
	qDebug() << program << arguments; // gilgil temp 2024.04.15
	QProcess::execute(program, arguments);

	//
	// Launch FireFox
	//
	{
		if (host.startsWith("."))
			host = host.mid(1);
#ifdef Q_OS_WIN
		QProcess::startDetached("firefox.exe", QStringList{host});
#else // Q_OS_WIN
#ifdef Q_OS_ANDROID
		QProcess::startDetached("am", QStringList{"start", "-n", "org.mozilla.firefox/org.mozilla.gecko.BrowserApp", host});
#else // Q_OS_ANDROID
		QProcess::startDetached("firefox", QStringList{host});
#endif // Q_OS_ANDROID
#endif // Q_OS_WIN
	}
}

void ChWidget::tbStart_clicked(bool checked) {
	(void)checked;

	if (!cookieHijack_.open()) {
		tbStop_->click();
		QMessageBox::warning(this, "Error", cookieHijack_.err->msg());
		return;
	}

	treeWidget_->clear();
	plainTextEdit_->clear();
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

#include "dbdialog.h"
void ChWidget::tbDb_clicked(bool checked) {
	(void)checked;

	bool dbOpened = cookieHijack_.cookieHijack_.active();
	if (!dbOpened) {
		if (!cookieHijack_.cookieHijack_.open()) {
			QMessageBox::warning(this, "Error", cookieHijack_.cookieHijack_.err->msg());
			cookieHijack_.cookieHijack_.close();
			return;
		}
	}

	DbDialog dbDialog(this, &cookieHijack_.cookieHijack_);
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
		cookieHijack_.cookieHijack_.close();
}

void ChWidget::tbFirefox_clicked(bool checked) {
	(void)checked;

	if (treeWidget_->selectedItems().count() == 0) return;
	GTreeWidgetItem* twi = PTreeWidgetItem(treeWidget_->selectedItems().at(0));
	QString host = twi->text(ColumnHost);
	QString cookie = twi->text(ColumnCookie);
	launchFirefox(host, cookie);
}

void ChWidget::tbScreenSaver_clicked(bool checked) {
	(void)checked;
	if (screenSaver_.active())
		screenSaver_.close();
	screenSaver_.open();
}

void ChWidget::treeWidget_itemSelectionChanged() {
	if (treeWidget_->selectedItems().count() > 0){
		QTreeWidgetItem* twi = treeWidget_->selectedItems().at(0);
		plainTextEdit_->setPlainText(twi->text(ColumnCookie));
	}
	setControl();
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
	jo["splitter"] >> GJson::splitterSizes(splitter_);
	jo["sizes"] >> GJson::columnSizes(treeWidget_);
}

void ChWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["ch"] << cookieHijack_;
	jo["splitter"] << GJson::splitterSizes(splitter_);
	jo["sizes"] << GJson::columnSizes(treeWidget_);
}
