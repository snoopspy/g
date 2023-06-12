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
			IpFlowValue* ipFlowValue = it.value();
			manager->ipFlowDeleted(ipFlowKey, ipFlowValue);
		}
	}
	flowMap_.clear();
	return GPacketMgr::doClose();
}

void GIpFlowMgr::deleteOldFlowMaps(long now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		IpFlowValue* ipFlowValue = it.value();
		long elapsed = now - ipFlowValue->lastTs_.tv_sec;
		long timeout = 0;
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
		currentIpFlowVal_->firstTs_ = currentIpFlowVal_->lastTs_ = packet->ts_;

		if (currentRevIpFlowVal_  == nullptr) {
			currentIpFlowVal_->state_ = IpFlowValue::Half;
		} else {
			currentIpFlowVal_->state_ = IpFlowValue::Full;
			currentRevIpFlowVal_->state_ = IpFlowValue::Full;
		}

		it = flowMap_.insert(currentIpFlowKey_, currentIpFlowVal_);
		for (Managable* manager: managables_)
			manager->ipFlowCreated(currentIpFlowKey_, currentIpFlowVal_);

	} else {
		currentIpFlowVal_ = it.value();
	}
	Q_ASSERT(currentIpFlowVal_ != nullptr);
	currentIpFlowVal_->lastTs_ = packet->ts_;

	emit managed(packet);
}
