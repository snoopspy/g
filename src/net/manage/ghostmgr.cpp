#include "ghostmgr.h"

// ----------------------------------------------------------------------------
// GHostMgr
// ----------------------------------------------------------------------------
bool GHostMgr::doOpen() {
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
	if (myIp_ =- 0) {
		SET_ERR(GErr::ValueIsZero, "ip is zero");
		return false;
	}
	myMac_ = intf_->mac();
	if (myMac_.isNull()) {
		SET_ERR(GErr::ValueIsZero, "mac is zero");
		return false;
	}
	gwIp_ = intf_->gateway();

	hostMap_.clear();
	return GPacketMgr::doOpen();
}

bool GHostMgr::doClose() {
	for (Managable* manager: managables_) {
		for (HostMap::iterator it = hostMap_.begin(); it != hostMap_.end(); it++) {
			GMac mac = it.key();
			GHostMgr::Value* value = it.value();
			manager->hostDeleted(mac, value);
		}
	}
	hostMap_.clear();
	return GPacketMgr::doClose();
}

void GHostMgr::deleteOldFlowMaps(long now) {
	HostMap::iterator it = hostMap_.begin();
	while (it != hostMap_.end()) {
		GHostMgr::Value* value = it.value();
		long elapsed = now - value->ts_.tv_sec;
		if (elapsed >= timeoutSec_) {
			GMac mac = it.key();
			qDebug() << QString("deleted %1 %2").arg(QString(mac)).arg(QString(it.value()->ip_)); // gilgil temp 2022.03.07
			for (Managable* manager: managables_)
				manager->hostDeleted(mac, value);
			it = hostMap_.erase(it);
			continue;
		}
		it++;
	}
}

bool GHostMgr::processArp(GEthHdr* ethHdr, GArpHdr* arpHdr, GMac* mac, GIp* ip) {
	if (ethHdr->smac() != arpHdr->smac()) {
		qDebug() << QString("ARP spoofing detected %1 %2 %3").arg(
			QString(ethHdr->smac()),
			QString(arpHdr->smac()),
			QString(arpHdr->sip()));
		return false;
	}

	*mac = arpHdr->smac();
	*ip = arpHdr->sip();
	return true;
}

bool GHostMgr::processIp(GEthHdr* ethHdr, GIpHdr* ipHdr, GMac* mac, GIp* ip) {
	GIp sip = ipHdr->sip();
	if (!intf_->isSameLanIp(sip)) return false;

	*mac = ethHdr->smac();
	*ip = sip;
	return true;
}

void GHostMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkIntervalSec_ != 0 && now - lastCheckTick_ >= checkIntervalSec_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
	}

	GMac mac;
	GIp ip;

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return;

	mac = ethHdr->smac();
	if (mac == myMac_) return;

	bool detected = false;
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr != nullptr && ipHdr->sip() != myIp_ && processIp(ethHdr, ipHdr, &mac, &ip))
		detected = true;

	GArpHdr* arpHdr = packet->arpHdr_;
	if (arpHdr != nullptr && arpHdr->sip() != myIp_ && processArp(ethHdr, arpHdr, &mac, &ip))
		detected = true;

	if (!detected) return;
	if (ip == myIp_ || ip == gwIp_) return;

	mac_ = mac;
	HostMap::iterator it = hostMap_.find(mac_);
	if (it == hostMap_.end()) {
		qDebug() << QString("detected %1 %2").arg(QString(mac)).arg(QString(ip)); // gilgil temp 2022.03.07
		val_ = reinterpret_cast<Value*>(GHostMgr::Value::allocate(requestItems_.totalMemSize_ + sizeof(GIp)));
		val_->ip_ = ip;
		it = hostMap_.insert(mac_, val_);
		for (Managable* manager: managables_)
			manager->hostDetected(mac_, val_);
	}
	else {
		val_ = it.value();
	}
	Q_ASSERT(val_ != nullptr);
	val_->ts_ = packet->ts_;

	emit managed(packet);
}
