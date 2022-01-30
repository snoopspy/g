#include "gpropitem-checkbox.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GPropItemCheckBox
// ----------------------------------------------------------------------------
GPropItemCheckBox::GPropItemCheckBox(GPropItemParam* param) : GPropItem(param) {
	checkBox_ = new GCheckBox(param->treeWidget_);
	param->treeWidget_->setItemWidget(item_, 1, checkBox_);
}

#endif // QT_GUI_LIB
