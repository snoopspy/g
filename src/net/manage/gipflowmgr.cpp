#include "gipflowmgr.h"

// ----------------------------------------------------------------------------
// GIpFlowMgr
// ----------------------------------------------------------------------------
bool GIpFlowMgr::doOpen() {
	flowMap_.clear();
	return GPacketMgr::doOpen();
}

bool GIpFlowMgr::doClose() {
	for (Managable* manager: managables_) {
		for (FlowMap::iterator it = flowMap_.begin(); it != flowMap_.end(); it++) {
			GFlow::IpFlowKey ipFlowKey = it.key();
			GPacketMgr::Value* value = it.value();
			manager->ipFlowDeleted(ipFlowKey, value);
		}
	}
	flowMap_.clear();
	return GPacketMgr::doClose();
}

void GIpFlowMgr::deleteOldFlowMaps(long now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		GPacketMgr::Value* value = it.value();
		long elapsed = now - value->ts_.tv_sec;
		long timeout = 0;
		switch (value->state_) {
			case GPacketMgr::Value::Half: timeout = halfTimeout_; break;
			case GPacketMgr::Value::Full: timeout = fullTimeout_; break;
			case GPacketMgr::Value::Rst: qCritical() << "unrecheable Rst"; timeout = 0; break;
			case GPacketMgr::Value::Fin: qCritical() << "unrecheable Fin"; timeout = 0; break;
		}
		if (elapsed >= timeout) {
			GFlow::IpFlowKey ipFlowKey = it.key();
			for (Managable* manager: managables_)
				manager->ipFlowDeleted(ipFlowKey, value);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GIpFlowMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkInterval_ != 0 && now - lastCheckTick_ >= checkInterval_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	ipFlowKey_.sip_ = ipHdr->sip();
	ipFlowKey_.dip_ = ipHdr->dip();
	FlowMap::iterator it = flowMap_.find(ipFlowKey_);

	rIpFlowKey_ = ipFlowKey_.reverse();
	rVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(rIpFlowKey_);
	if (rIt != flowMap_.end())
		rVal_ = rIt.value();

	if (it == flowMap_.end()) {
		val_ = GPacketMgr::Value::allocate(GPacketMgr::Value::Half, requestItems_.totalMemSize_);
		it = flowMap_.insert(ipFlowKey_, val_);
		for (Managable* manager: managables_)
			manager->ipFlowDetected(ipFlowKey_, val_);

		if (rVal_ != nullptr) {
			val_->state_ = GPacketMgr::Value::Full;
			rVal_->state_ = GPacketMgr::Value::Full;
		}
	} else {
		val_ = it.value();
	}
	Q_ASSERT(val_ != nullptr);
	val_->ts_ = packet->ts_;

	emit managed(packet);
}
