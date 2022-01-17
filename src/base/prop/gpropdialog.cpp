#include "gpropdialog.h"

#ifdef QT_GUI_LIB

#include <QVBoxLayout>
#include "base/gjson.h"

// ----------------------------------------------------------------------------
// GPropDialog
// ----------------------------------------------------------------------------
GPropDialog::GPropDialog(QWidget *parent) : QDialog(parent) {
	resize(QSize(640, 480));
	QLayout* layout = new QVBoxLayout;
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(&widget_);
	setLayout(layout);
}

GPropDialog::~GPropDialog() {
}

void GPropDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
	jo["widget"] >> widget_;
}

void GPropDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
	jo["widget"] << widget_;
}

#endif
