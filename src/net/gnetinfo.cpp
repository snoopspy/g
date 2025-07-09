#include "net/gnetinfo.h"

// ----------------------------------------------------------------------------
// GNetInfo
// ----------------------------------------------------------------------------
void GNetInfo::init() {
	intfList_.init();
	rtm_.init();
	for (int i = 0; i < rtm_.count(); i++) {
		GRtmEntry& entry = const_cast<GRtmEntry&>(rtm_.at(i));
		QString intfName = entry.intfName_;
		GIntf* intf = intfList_.findByName(intfName);
		if (intf == nullptr) {
			QString msg = QString("interfaceList_.findByName(%1) return false").arg(intfName);
			qCritical() << msg;
		}
		entry.intf_ = intf;
	}

	for (GIntf& intf: intfList_) {
		intf.gateway_ = rtm_.findGateway(intf.name_, intf.ip_);
	}
}

void GNetInfo::clear() {
	intfList_.clear();
	rtm_.clear();
}
