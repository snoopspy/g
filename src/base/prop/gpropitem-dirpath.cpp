#include "gpropitem-dirpath.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GPropItemDirPath
// ----------------------------------------------------------------------------
GPropItemDirPath::GPropItemDirPath(GPropItemParam* param) : GPropItemWidget(param) {
	layout_ = new QHBoxLayout;
	lineEdit_ = new QLineEdit(param->treeWidget_);
	toolButton_ = new QToolButton(param->treeWidget_);
    layout_->setContentsMargins(0, 0, 0, 0);
	lineEdit_->setFrame(false);
	toolButton_->setText("...");
	layout_->addWidget(lineEdit_);
	layout_->addWidget(toolButton_);
	widget_->setLayout(layout_);

	QObject::connect(lineEdit_, SIGNAL(editingFinished()), this, SLOT(myEditingFinished()));
	QObject::connect(toolButton_, SIGNAL(clicked(bool)), this, SLOT(myToolButtonClicked(bool)));
}

void GPropItemDirPath::update() {
	QVariant value = object_->property(mpro_.name());
	lineEdit_->setText(value.toString());
}

void GPropItemDirPath::myEditingFinished() {
	bool res = object_->setProperty(mpro_.name(), QVariant::fromValue<QString>(lineEdit_->text()));
	if (!res) {
		qWarning() << QString("object->setProperty(%1, %2) return false").arg(mpro_.name(), lineEdit_->text());
	}
	update();
}

#include <QFileDialog>
void GPropItemDirPath::myToolButtonClicked(bool checked) {
	(void)checked;
	QFileDialog fd;
    fd.setFileMode(QFileDialog::Directory);
	fd.setOption(QFileDialog::ShowDirsOnly);
	if (fd.exec()) {
		bool res = object_->setProperty(mpro_.name(), QVariant::fromValue<QString>(fd.selectedFiles().at(0)));
		if (!res) {
			qWarning() << QString("object->setProperty(%1, %2) return false").arg(mpro_.name(), lineEdit_->text());
		}
		update();
	}
}

#endif // QT_GUI_LIB
