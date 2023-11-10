#include "gpropwidget.h"

#ifdef QT_GUI_LIB

#include <QMessageBox>
#include "base/gjson.h"
#include "base/gstateobj.h"

// ----------------------------------------------------------------------------
// GPropWidget
// ----------------------------------------------------------------------------
GPropWidget::GPropWidget(QWidget *parent) : QWidget(parent) {
	resize(QSize(640, 480));
	init();
	setControl();
}

GPropWidget::~GPropWidget() {
	clear();
}

void GPropWidget::init() {
	// (actionOpen_ = new QAction(this))->setText("Open"); // gilgil temp 2020.05.21
	// (actionClose_ = new QAction(this))->setText("Close"); // gilgil temp 2020.05.21

	mainLayout_ = new QVBoxLayout(this);
	mainLayout_->setContentsMargins(0, 0, 0, 0);
	mainLayout_->setSpacing(0);

	// ----- gilgil temp 2020.05.21 -----
	/*
	toolBar_ = new QToolBar(this);
	toolBar_->addAction(actionOpen_);
	toolBar_->addAction(actionClose_);
	*/
	// ----------------------------------

	treeWidget_ = new GTreeWidget(this);
	treeWidget_->setColumnCount(2);
	treeWidget_->setHeaderLabels(QStringList() << "Property" << "Value");

	// mainLayout_->addWidget(toolBar_, 0); // gilgil temp 2020.05.21
	mainLayout_->addWidget(treeWidget_, 1);
	this->setLayout(mainLayout_);

	// QObject::connect(actionOpen_, &QAction::triggered, this, &GPropWidget::actionOpenTriggered); // gilgil temp 2020.05.21
	// QObject::connect(actionClose_, &QAction::triggered, this, &GPropWidget::actionCloseTriggered); // gilgil temp 2020.05.21
}

void GPropWidget::setObject(QObject* object) {
	if (object == object_) return;
	object_ = object;
	clear();
	if (object == nullptr) return;

	GProp* prop = dynamic_cast<GProp*>(object_);
	if (prop == nullptr) {
		qWarning() << "prop is nullptr. object must be descendant of both QObject and GProp";
		return;
	}
	prop->propCreateItems(treeWidget_, nullptr, object_);

	update();
}

void GPropWidget::update() {
	QObjectList list = treeWidget_->children();
	for (QObject* object: list) {
		GPropItem* item = dynamic_cast<GPropItem*>(object);
		if (item != nullptr)
			item->update();
	}
	treeWidget_->update();
	setControl();
}

void GPropWidget::clear() {
	QObjectList list = treeWidget_->children();
	for (QObject* object: list) {
		GPropItem* item = dynamic_cast<GPropItem*>(object);
		if (item != nullptr)
			delete item;
	}
	treeWidget_->clear();
	setControl();
}

void GPropWidget::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["sizes"] >> GJson::columnSizes(treeWidget_);
}

void GPropWidget::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["sizes"] << GJson::columnSizes(treeWidget_);
}

void GPropWidget::setControl() {
	GStateObj* stateObj = object_ == nullptr ? nullptr : dynamic_cast<GStateObj*>(object_);
	// toolBar_->setEnabled(stateObj != nullptr); // gilgil temp 2020.05.21

	bool active = false;
	if (stateObj != nullptr)
		active = stateObj->active();
	// actionOpen_->setEnabled(!active); // gilgil temp 2020.05.21
	// actionClose_->setEnabled(active); // gilgil temp 2020.05.21
	treeWidget_->setEnabled(!active);
}

// ----- gilgil temp 2020.05.21 -----
/*
void GPropWidget::actionOpenTriggered(bool) {
	GStateObj* stateObj = dynamic_cast<GStateObj*>(object_);
	Q_ASSERT(stateObj != nullptr);
	bool res = stateObj->open();
	if (!res) {
		QString msg = stateObj->err->msg();
		QMessageBox::warning(this, "Error", msg);
	}
	setControl();
}

void GPropWidget::actionCloseTriggered(bool) {
	GStateObj* stateObj = dynamic_cast<GStateObj*>(object_);
	Q_ASSERT(stateObj != nullptr);
	stateObj->close();
	setControl();
}
*/
// ----------------------------------

#endif // QT_GUI_LIB
