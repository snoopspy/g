#include "ghostmgr.h"

// ----------------------------------------------------------------------------
// GHostMgr
// ----------------------------------------------------------------------------
bool GHostMgr::doOpen() {
	flowMap_.clear();
	return GPacketMgr::doOpen();
}

bool GHostMgr::doClose() {
	for (Managable* manager: managables_) {
		for (FlowMap::iterator it = flowMap_.begin(); it != flowMap_.end(); it++) {
			GMac mac = it.key();
			GPacketMgr::Value* value = it.value();
			manager->hostDeleted(mac, value);
		}
	}
	flowMap_.clear();
	return GPacketMgr::doClose();
}

void GHostMgr::deleteOldFlowMaps(long now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		GPacketMgr::Value* value = it.value();
		long elapsed = now - value->ts_.tv_sec;
		if (elapsed >= timeout_) {
			GMac mac = it.key();
			for (Managable* manager: managables_)
				manager->hostDeleted(mac, value);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GHostMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkInterval_ != 0 && now - lastCheckTick_ >= checkInterval_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
	}

	switch (packet->dlt()) {
		case GPacket::Eth: {
			GEthHdr* ethHdr = packet->ethHdr_;
			if (ethHdr == nullptr) {
				qWarning() << "ethHdr is null";
				return;
			}
			mac_ = ethHdr->smac();
			break;
		}
		case GPacket::Ip:
			qWarning() << "not supported dlt(GPacket::Ip)";
			return;
		case GPacket::Dot11: {
			GDot11Hdr* dot11Hdr = packet->dot11Hdr_;
			if (dot11Hdr == nullptr)
				return;
			mac_ = dot11Hdr->ta();
			break;
		}
		case GPacket::Null:
			qWarning() << "not supported dlt(GPacket::Null)";
			return;
	}

	FlowMap::iterator it = flowMap_.find(mac_);
	if (it == flowMap_.end()) {
		val_ = GPacketMgr::Value::allocate(GPacketMgr::Value::Full /*meaninglesss*/, requestItems_.totalMemSize_);
		it = flowMap_.insert(mac_, val_);
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
