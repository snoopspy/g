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
	if (checkIntervalSec_ != 0 && now - lastCheckTick_ >= checkIntervalSec_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	currentIpFlowKey_.sip_ = ipHdr->sip();
	currentIpFlowKey_.dip_ = ipHdr->dip();
	FlowMap::iterator it = flowMap_.find(currentIpFlowKey_);

	currentRevIpFlowKey_ = currentIpFlowKey_.reverse();
	currentRevVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(currentRevIpFlowKey_);
	if (rIt != flowMap_.end())
		currentRevVal_ = rIt.value();

	if (it == flowMap_.end()) {
		currentVal_ = GPacketMgr::Value::allocate(GPacketMgr::Value::Half, requestItems_.totalMemSize_);
		it = flowMap_.insert(currentIpFlowKey_, currentVal_);
		for (Managable* manager: managables_)
			manager->ipFlowDetected(currentIpFlowKey_, currentVal_);

		if (currentRevVal_ != nullptr) {
			currentVal_->state_ = GPacketMgr::Value::Full;
			currentRevVal_->state_ = GPacketMgr::Value::Full;
		}
	} else {
		currentVal_ = it.value();
	}
	Q_ASSERT(currentVal_ != nullptr);
	currentVal_->ts_ = packet->ts_;

	emit managed(packet);
}
