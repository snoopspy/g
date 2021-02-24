#include "gpropitem-device.h"
#include "net/gnetinfo.h"

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GPropItemDevice
// ----------------------------------------------------------------------------
GPropItemDevice::GPropItemDevice(GPropItemParam* param) : GPropItemComboBox(param) {
	GAllInterface& allInterface = GNetInfo::instance().allInterface();
	for (int i = 0; i < allInterface.count(); i++) {
		const GInterface& intf = allInterface.at(i);
#ifdef Q_OS_LINUX
		QString s = intf.name();
#endif
#ifdef Q_OS_WIN
		QString s = intf.desc();
#endif
		comboBox_->addItem(s);
		devNames_.push_back(intf.name());
	}
	QObject::connect(comboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(myCurrentIndexChanged(int)));
}

void GPropItemDevice::update() {
	QString devName = object_->property(mpro_.name()).toString();
	GAllInterface& allInterface = GNetInfo::instance().allInterface();
	for (int i = 0; i < allInterface.count(); i++) {
		const GInterface& intf = allInterface.at(i);
		if (intf.name() == devName) {
			comboBox_->setCurrentIndex(i);
			return;
		}
	}
	comboBox_->setCurrentIndex(-1);
}

void GPropItemDevice::myCurrentIndexChanged(int index) {
	if (index == -1) return;
	Q_ASSERT(index <= devNames_.count());
	QString devName = devNames_.at(index);
	bool res = object_->setProperty(mpro_.name(), devName);
	if (!res) {
		qWarning() << QString("object->setProperty(%1, %2) return false").arg(mpro_.name(), devName);
	}
}

#endif // QT_GUI_LIB
