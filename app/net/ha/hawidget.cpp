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
	treeWidget_->setHeaderLabels(QStringList{"IP", "Name", "Duration", ""});
	treeWidget_->setSortingEnabled(true);
	treeWidget_->sortByColumn(-1, Qt::AscendingOrder);
	treeWidget_->setIndentation(0);
	treeWidget_->setItemDelegate(itemDelegate_);

	QHeaderView* hv = treeWidget_->header();
	hv->setSectionResizeMode(2, QHeaderView::Stretch);
	hv->setSectionResizeMode(3, QHeaderView::Fixed);
	hv->setStretchLastSection(false);
	treeWidget_->setColumnWidth(3, ItemHeight);

	mainLayout_->addWidget(treeWidget_);

	int left, top, right, bottom;
	mainLayout_->getContentsMargins(&left, &top, &right, &bottom);
	mainLayout_->setContentsMargins(0, top, 0, 0);

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
	bool isFirst = jo.find("propDialog") == jo.end();
	jo["propDialog"] >> propDialog;

#ifndef Q_OS_ANDROID
	propDialog.show();
#else
	propDialog.showMaximized();
#endif
	if (isFirst) {
		int width = propDialog.width();
		propDialog.widget_.treeWidget_->setColumnWidth(0, width / 2);
	}

	propDialog.exec();

	jo["propDialog"] << propDialog;
}

void HaWidget::processClosed() {
	if (hostAnalyzer_.active())
		tbStop_->click();
}
