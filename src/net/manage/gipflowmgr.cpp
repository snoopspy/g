#include "gipflowmgr.h"

// ----------------------------------------------------------------------------
// GIpFlowMgr
// ----------------------------------------------------------------------------
bool GIpFlowMgr::doOpen() {
	flowMap_.clear();
	return GPktMgr::doOpen();
}

bool GIpFlowMgr::doClose() {
	for (Managable* manager: managables_) {
		for (FlowMap::iterator it = flowMap_.begin(); it != flowMap_.end(); it++) {
			GFlow::IpFlowKey key = it.key();
			GPktMgr::Value* value = it.value();
			manager->ipFlowDeleted(&key, value);
		}
	}
	flowMap_.clear();
	return GPktMgr::doClose();
}

void GIpFlowMgr::deleteOldFlowMaps(long now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		GPktMgr::Value* value = it.value();
		long elapsed = now - value->ts_.tv_sec;
		long timeout = 0;
		switch (value->state_) {
			case GPktMgr::Value::Half: timeout = halfTimeout_; break;
			case GPktMgr::Value::Full: timeout = fullTimeout_; break;
			case GPktMgr::Value::Rst: qCritical() << "unrecheable Rst"; timeout = 0; break;
			case GPktMgr::Value::Fin: qCritical() << "unrecheable Fin"; timeout = 0; break;
		}
		if (elapsed >= timeout) {
			GFlow::IpFlowKey* key = const_cast<GFlow::IpFlowKey*>(&it.key());
			for (Managable* manager: managables_)
				manager->ipFlowDeleted(key, value);
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

	key_.sip_ = ipHdr->sip();
	key_.dip_ = ipHdr->dip();
	FlowMap::iterator it = flowMap_.find(key_);

	rKey_ = key_.reverse();
	rVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(rKey_);
	if (rIt != flowMap_.end())
		rVal_ = rIt.value();

	if (it == flowMap_.end()) {
		val_ = GPktMgr::Value::allocate(GPktMgr::Value::Half, requestItems_.totalMemSize_);
		it = flowMap_.insert(key_, val_);
		for (Managable* manager: managables_)
			manager->ipFlowCreated(&key_, val_);

		if (rVal_ != nullptr) {
			val_->state_ = GPktMgr::Value::Full;
			rVal_->state_ = GPktMgr::Value::Full;
		}
	} else {
		val_ = it.value();
	}
	Q_ASSERT(val_ != nullptr);
	val_->ts_ = packet->ts_;

	emit managed(packet);
}
