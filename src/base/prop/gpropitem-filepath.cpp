#include "gpropitem-filepath.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GPropItemFilePath
// ----------------------------------------------------------------------------
GPropItemFilePath::GPropItemFilePath(GPropItemParam* param) : GPropItemWidget(param) {
	layout_ = new QHBoxLayout;
	lineEdit_ = new QLineEdit(param->treeWidget_);
	toolButton_ = new QToolButton(param->treeWidget_);
	fd_ = new QFileDialog(param->treeWidget_);
	layout_->setContentsMargins(0, 0, 0, 0);
	lineEdit_->setFrame(false);
	toolButton_->setText("...");
	toolButton_->setToolTip("Select File");
	layout_->addWidget(lineEdit_);
	layout_->addWidget(toolButton_);
	widget_->setLayout(layout_);

	QObject::connect(lineEdit_, SIGNAL(editingFinished()), this, SLOT(myEditingFinished()));
	QObject::connect(toolButton_, SIGNAL(clicked(bool)), this, SLOT(myToolButtonClicked(bool)));
}

void GPropItemFilePath::update() {
	QVariant value = object_->property(mpro_.name());
	lineEdit_->setText(value.toString());
}

void GPropItemFilePath::myEditingFinished() {
	bool res = object_->setProperty(mpro_.name(), QVariant::fromValue<QString>(lineEdit_->text()));
	if (!res) {
		qWarning() << QString("object->setProperty(%1, %2) return false").arg(mpro_.name(), lineEdit_->text());
	}
	update();
}

QString _prevDirectory;
void GPropItemFilePath::myToolButtonClicked(bool checked) {
	(void)checked;
	fd_->setFileMode(QFileDialog::AnyFile);
	if (_prevDirectory != "")
		fd_->setDirectory(_prevDirectory);
	else {
		QString filePath = object_->property(mpro_.name()).toString();
		if (filePath != "") {
			QFileInfo fi = QFileInfo(filePath);
			fd_->setDirectory(fi.absolutePath());
		}
	}
	if (fd_->exec()) {
		QFileInfo fi = QFileInfo(fd_->selectedFiles().at(0));
		_prevDirectory = fi.absolutePath();
		bool res = object_->setProperty(mpro_.name(), fi.filePath());
		if (!res) {
			qWarning() << QString("object->setProperty(%1, %2) return false").arg(mpro_.name(), lineEdit_->text());
		}
		update();
	}
}

#endif // QT_GUI_LIB
