#include "gipflowmgr.h"

// ----------------------------------------------------------------------------
// GIpFlowMgr
// ----------------------------------------------------------------------------
bool GIpFlowMgr::doOpen() {
	flowMap_.clear();
	return GPacketMgr::doOpen();
}

bool GIpFlowMgr::doClose() {
	while (true) {
		if (flowMap_.count() == 0) break;
		FlowMap::iterator oldestIt = flowMap_.begin();
		GFlow::IpFlowKey oldestIpFlowKey = oldestIt.key();
		IpFlowValue* oldestIpFlowValue = oldestIt.value();
		FlowMap::iterator it = oldestIt;
		it++;
		for (; it != flowMap_.end(); it++) {
			IpFlowValue* ipFlowValue = it.value();
			if (oldestIpFlowValue->lastTime_.tv_sec > ipFlowValue->lastTime_.tv_sec) {
				oldestIt = it;
				oldestIpFlowKey = it.key();
				oldestIpFlowValue = it.value();
			}
		}
		for (Managable* manager: managables_)
			manager->ipFlowDeleted(oldestIpFlowKey, oldestIpFlowValue);
		flowMap_.erase(oldestIt);
	}
	Q_ASSERT(flowMap_.count() == 0);
	return GPacketMgr::doClose();
}

void GIpFlowMgr::deleteOldFlowMaps(time_t now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		IpFlowValue* ipFlowValue = it.value();
		time_t elapsed = now - ipFlowValue->lastTime_.tv_sec;
		time_t timeout = 0;
		switch (ipFlowValue->state_) {
			case IpFlowValue::Half: timeout = halfTimeout_; break;
			case IpFlowValue::Full: timeout = fullTimeout_; break;
		}
		if (elapsed >= timeout) {
			GFlow::IpFlowKey ipFlowKey = it.key();
			for (Managable* manager: managables_)
				manager->ipFlowDeleted(ipFlowKey, ipFlowValue);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GIpFlowMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkIntervalSec_ != 0 && now - lastCheckClock_ >= checkIntervalSec_) {
		deleteOldFlowMaps(now);
		lastCheckClock_ = now;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	currentIpFlowKey_.sip_ = ipHdr->sip();
	currentIpFlowKey_.dip_ = ipHdr->dip();
	FlowMap::iterator it = flowMap_.find(currentIpFlowKey_);

	currentRevIpFlowKey_ = currentIpFlowKey_.reverse();
	currentRevIpFlowVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(currentRevIpFlowKey_);
	if (rIt != flowMap_.end())
		currentRevIpFlowVal_ = rIt.value();

	if (it == flowMap_.end()) {
		currentIpFlowVal_ = IpFlowValue::allocate(requestItems_.totalMemSize_);
		currentIpFlowVal_->firstTime_ = currentIpFlowVal_->lastTime_ = packet->ts_;

		if (currentRevIpFlowVal_ == nullptr) {
			currentIpFlowVal_->state_ = IpFlowValue::Half;
			currentIpFlowVal_->direction_ = ClientToServer;
		} else {
			currentIpFlowVal_->state_ = IpFlowValue::Full;
			currentIpFlowVal_->direction_ = ServerToClient;
			currentRevIpFlowVal_->state_ = IpFlowValue::Full;
		}

		it = flowMap_.insert(currentIpFlowKey_, currentIpFlowVal_);
		currentPacket_ = packet;
		for (Managable* manager: managables_)
			manager->ipFlowCreated(currentIpFlowKey_, currentIpFlowVal_);

	} else {
		currentIpFlowVal_ = it.value();
	}
	Q_ASSERT(currentIpFlowVal_ != nullptr);
	currentIpFlowVal_->lastTime_ = packet->ts_;
	currentIpFlowVal_->packets_++;
	currentIpFlowVal_->bytes_ += packet->buf_.size_;

	emit managed(packet);
}
