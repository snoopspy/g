#include "gipmgr.h"

// ----------------------------------------------------------------------------
// GIpMgr
// ----------------------------------------------------------------------------
bool GIpMgr::doOpen() {
	if (pcapDevice_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "pcapDevice is null");
		return false;
	}

	intf_ = pcapDevice_->intf();
	if (intf_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "intf is null");
		return false;
	}

	myIp_ = intf_->ip();
	if (myIp_ == 0) {
		SET_ERR(GErr::ValueIsZero, "ip is zero");
		return false;
	}

	{
		QMutexLocker ml(&ipMap_);
		ipMap_.clear();
	}
	return GPacketMgr::doOpen();
}

bool GIpMgr::doClose() {
	{
		QMutexLocker ml(&ipMap_);
		for (Managable* manager: managables_) {
			for (IpMap::iterator it = ipMap_.begin(); it != ipMap_.end(); it++) {
				GIp ip = it.key();
				IpValue* ipValue = it.value();
				manager->ipDeleted(ip, ipValue);
			}
		}
		ipMap_.clear();
	}
	return GPacketMgr::doClose();
}

void GIpMgr::deleteOldHosts(time_t now) {
	QMutexLocker ml(&ipMap_);
	IpMap::iterator it = ipMap_.begin();
	while (it != ipMap_.end()) {
		IpValue* ipValue = it.value();
		long elapsed = now - ipValue->lastTime_;
		if (elapsed >= timeoutSec_) {
			GIp ip = it.key();
			qDebug() << QString("%1").arg(QString(ip));
			for (Managable* manager: managables_)
				manager->ipDeleted(ip, ipValue);
			it = ipMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GIpMgr::processIp(GPacket* packet, GIp ip) {
	if (ip == myIp_) return;
	if (ip.isLocalHost()) return;
	if (ip.isMulticast()) return;
	if (ip.isBroadcast()) return;

	switch (category_) {
		case Lan :
			if (!intf_->isSameLanIp(ip)) return;
			break;
		case Wan :
			if (intf_->isSameLanIp(ip)) return;
			break;
		case All :
			break;
	}

	currentIp_ = ip;
	{
		QMutexLocker ml(&ipMap_);
		IpMap::iterator it = ipMap_.find(currentIp_);
		if (it == ipMap_.end()) {
			qDebug() << QString("detected %1").arg(QString(ip));
			currentIpVal_ = IpValue::allocate(requestItems_.totalMemSize_);
			currentIpVal_->firstTime_ = currentIpVal_->lastTime_ = packet->ts_.tv_sec;

			it = ipMap_.insert(currentIp_, currentIpVal_);
			for (Managable* manager: managables_)
				manager->ipCreated(currentIp_, currentIpVal_);
		}
	}
	Q_ASSERT(currentIpVal_ != nullptr);
	currentIpVal_->lastTime_ = packet->ts_.tv_sec;
}

void GIpMgr::manage(GPacket* packet) {
	time_t now = packet->ts_.tv_sec;
	if (checkIntervalSec_ != 0 && now - lastCheckClock_ >= checkIntervalSec_) {
		deleteOldHosts(now);
		lastCheckClock_ = now;
	}

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return;

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	// processIp(packet, ipHdr->sip()); // gilgil temp 2023.12.04
	processIp(packet, ipHdr->dip());

	emit managed(packet);
}

bool GIpMgr::propLoad(QJsonObject jo, QMetaProperty mpro) {
	if (QString(mpro.name()) == "pcapDevice") {
		QObject* p = parent();
		if (p != nullptr && QString(p->metaObject()->className()) == "GAutoArpSpoof")
			return true;
	}
	return GStateObj::propLoad(jo, mpro);
}
