#include "hawidget.h"

#include <QItemDelegate>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>

#include <GJson>

struct ItemDelegate : public QItemDelegate {
private:
	int height_;

public:
	ItemDelegate(QObject *parent = nullptr, int height = -1) : QItemDelegate(parent), height_(height) {}

	void setHeight(int height) { height_ = height; }

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
		QSize size = QItemDelegate::sizeHint(option, index);
		if (height_ != -1)
			size.setHeight(height_);
		return size;
	}
};

HaWidget::HaWidget(QWidget* parent) : GDefaultWidget(parent) {
	setWindowTitle("Host Analyzer");

	itemDelegate_ = new ItemDelegate;
	itemDelegate_->setHeight(ItemHeight);

	treeWidget_ = new GTreeWidget(this);
	treeWidget_->setHeaderLabels(QStringList{"IP", "Name", "Elapse", ""});
	treeWidget_->setSortingEnabled(true);
	treeWidget_->sortByColumn(-1, Qt::AscendingOrder);
	treeWidget_->setIndentation(0);
	treeWidget_->setItemDelegate(itemDelegate_);

	QHeaderView* hv = treeWidget_->header();
	hv->setSectionResizeMode(2, QHeaderView::Stretch);
	hv->setSectionResizeMode(3, QHeaderView::Fixed);
	hv->setStretchLastSection(false);
	treeWidget_->setColumnWidth(3, ItemHeight);

	tbDb_ = new QToolButton(this);
	tbDb_->setText("DB");
	tbDb_->setIcon(QIcon(":/img/db.png"));
	tbDb_->setAutoRaise(true);
	tbDb_->setIconSize(tbStart_->iconSize());
	toolButtonLayout_->addWidget(tbDb_);

	mainLayout_->addWidget(treeWidget_);

	dbDialog_ = new DbDialog(this);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

	QObject::connect(tbStart_, &QToolButton::clicked, this, &HaWidget::tbStart_clicked);
	QObject::connect(tbStop_, &QToolButton::clicked, this, &HaWidget::tbStop_clicked);
	QObject::connect(tbOption_, &QToolButton::clicked, this, &HaWidget::tbOption_clicked);
	QObject::connect(tbDb_, &QToolButton::clicked, this, &HaWidget::tbDb_clicked);

	hostAnalyzer_.treeWidget_ = treeWidget_;

	setControl();
}

HaWidget::~HaWidget() {
	tbStop_->click();
	setControl();

	if (dbDialog_ != nullptr) {
		delete dbDialog_;
		dbDialog_ = nullptr;
	}
}

void HaWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["sizes"] >> GJson::columnSizes(treeWidget_);
	jo["ha"] >> hostAnalyzer_;
	jo["dbdialog"] >> *dbDialog_;
}

void HaWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["sizes"] << GJson::columnSizes(treeWidget_);
	jo["ha"] << hostAnalyzer_;
	jo["dbdialog"] << *dbDialog_;
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

void HaWidget::tbDb_clicked(bool checked) {
	(void)checked;

#ifdef Q_OS_ANDROID
	dbDialog_->showMaximized();
#else // Q_OS_ANDROID
	dbDialog_->show();
#endif // Q_OS_ANDROID
	dbDialog_->exec();

	// Change default name because alias can be chaned
	int count = treeWidget_->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		GTreeWidgetItem* item = dynamic_cast<GTreeWidgetItem*>(treeWidget_->topLevelItem(i));
		Q_ASSERT(item != nullptr);
		QString mac = item->property("mac").toString();
		QString defaultName = hostAnalyzer_.hostDb_.getDefaultName(mac, nullptr);
		item->setText(1, defaultName);
	}
}

void HaWidget::processClosed() {
	if (hostAnalyzer_.active())
		tbStop_->click();
}
