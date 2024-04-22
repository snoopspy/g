#include "gpropitem-interface.h"
#include "net/gnetinfo.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GPropItemInterface
// ----------------------------------------------------------------------------
GPropItemInterface::GPropItemInterface(GPropItemParam* param, bool editable) : GPropItemComboBox(param) {
	comboBox_->setEditable(editable);
	GIntfList& intfList = GNetInfo::instance().intfList();
	for (int i = 0; i < intfList.count(); i++) {
		const GIntf& intf = intfList.at(i);
#ifdef Q_OS_LINUX
		QString s = intf.name();
#endif
#ifdef Q_OS_WIN
		QString s = intf.desc();
#endif
		comboBox_->addItem(s);
		intfNames_.push_back(intf.name());
	}
	QObject::connect(comboBox_, &QComboBox::currentTextChanged, this, &GPropItemInterface::myCurrentTextChanged);
	QObject::connect(comboBox_, &QComboBox::currentIndexChanged, this, &GPropItemInterface::myCurrentIndexChanged);
}

void GPropItemInterface::update() {
	QString intfName = object_->property(mpro_.name()).toString();
	GIntfList& intfList = GNetInfo::instance().intfList();
	for (int i = 0; i < intfList.count(); i++) {
		const GIntf& intf = intfList.at(i);
		if (intf.name() == intfName) {
			comboBox_->setCurrentIndex(i);
			return;
		}
	}
	comboBox_->setCurrentIndex(-1);
	comboBox_->setCurrentText(intfName);
}

void GPropItemInterface::myCurrentTextChanged(const QString &text) {
	bool res = object_->setProperty(mpro_.name(), text);
	if (!res) {
		qWarning() << QString("object->setProperty(%1, %2) return false").arg(mpro_.name(), text);
	}
}

void GPropItemInterface::myCurrentIndexChanged(int index) {
	if (index == -1) return;
	QString text;
	if (index < intfNames_.count())
		text = intfNames_.at(index);
	else
		text = comboBox_->currentText();
	myCurrentTextChanged(text);
}

#endif // QT_GUI_LIB
