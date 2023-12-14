#include "base/gobj.h"
#include "gpropitem-objrefarrayptr.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GPropItemObjRefArrayMgr
// ----------------------------------------------------------------------------
struct GPropItemObjRefArrayMgr : QObject, QTreeWidgetItem {
	Q_OBJECT

public:
	GPropItemObjRefArrayMgr(QTreeWidgetItem* parent, GObjRefArrayPtr array, GObj* obj)
		: QObject(parent->treeWidget()), QTreeWidgetItem(parent), array_(array), obj_(obj) {
		int index = array->indexOf(GObjRef(obj));
		Q_ASSERT(index != -1);
		QString text = QString("item%1").arg(index);
		this->setText(0, text);
		this->setExpanded(true);

		QTreeWidget* treeWidget = parent->treeWidget();
		Q_ASSERT(treeWidget != nullptr);
		QToolButton* toolButton = new QToolButton(treeWidget);
		toolButton->setText("-");
		toolButton->setToolTip("Remove");
		toolButton->setAutoRaise(true);
		treeWidget->setItemWidget(this, 1, toolButton);
		QObject::connect(toolButton, SIGNAL(clicked(bool)), this, SLOT(myClicked(bool)));
	}

	GObjRefArrayPtr array_;
	GObj* obj_;

protected slots:
	void myClicked(bool checked) {
		(void)checked;
		array_->delObj(obj_);
		delete this;
	}
};

// ----------------------------------------------------------------------------
// GPropItemObjRefArrayPtr
// ----------------------------------------------------------------------------
GPropItemObjRefArrayPtr::GPropItemObjRefArrayPtr(GPropItemParam* param) : GPropItemToolButton(param) {
	toolButton_->setText("+");
	toolButton_->setToolTip("Add");
	QObject::connect(toolButton_, SIGNAL(clicked(bool)), this, SLOT(myClicked(bool)));
}

void GPropItemObjRefArrayPtr::update() {
	while (item_->childCount() > 0) item_->removeChild(item_->child(0));
	const char* propName = mpro_.name();
	QVariant variant = object_->property(propName);
	GObjRefArrayPtr array = qvariant_cast<GObjRefArrayPtr>(variant);
	for (_GObjRefArray::iterator it = array->begin(); it != array->end(); it++) {
		GObj* childObj = *it;
		GPropItemObjRefArrayMgr* mgr = new GPropItemObjRefArrayMgr(item_, array, childObj);
		childObj->propCreateItems(item_->treeWidget(), mgr, childObj);
	}
}

void GPropItemObjRefArrayPtr::myClicked(bool checked) {
	(void)checked;
	const char* propName = mpro_.name();
	QVariant variant = object_->property(propName);
	GObjRefArrayPtr array = qvariant_cast<GObjRefArrayPtr>(variant);
	Q_ASSERT(array != nullptr);
	GObj* childObj = array->addObj();
	Q_ASSERT(childObj != nullptr);
	GPropItemObjRefArrayMgr* mgr = new GPropItemObjRefArrayMgr(item_, array, childObj);
	childObj->propCreateItems(item_->treeWidget(), mgr, childObj);
}

#include "gpropitem-objrefarrayptr.moc"

#endif // QT_GUI_LIB
