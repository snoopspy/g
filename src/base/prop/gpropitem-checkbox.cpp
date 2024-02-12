#include "gpropitem-checkbox.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GPropItemCheckBox
// ----------------------------------------------------------------------------
GPropItemCheckBox::GPropItemCheckBox(GPropItemParam* param) : GPropItem(param) {
	checkBox_ = new GCheckBox(param->treeWidget_);
	QString blankString;
	for (int i = 0; i < 1024; i++)
		blankString += ' ';
	checkBox_->setText(blankString);
	param->treeWidget_->setItemWidget(item_, 1, checkBox_);
}

#endif // QT_GUI_LIB
