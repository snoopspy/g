#include "gudpflowmgr.h"

// ----------------------------------------------------------------------------
// GUdpFlowMgr
// ----------------------------------------------------------------------------
bool GUdpFlowMgr::doOpen() {
	flowMap_.clear();
	return GPacketMgr::doOpen();
}

bool GUdpFlowMgr::doClose() {
	for (Managable* manager: managables_) {
		for (FlowMap::iterator it = flowMap_.begin(); it != flowMap_.end(); it++) {
			GFlow::UdpFlowKey udpFlowKey = it.key();
			UdpFlowValue* udpFlowValue = it.value();
			manager->udpFlowDeleted(udpFlowKey, udpFlowValue);
		}
	}
	flowMap_.clear();
	return GPacketMgr::doClose();
}

void GUdpFlowMgr::deleteOldFlowMaps(time_t now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		UdpFlowValue* udpFlowValue = it.value();
		time_t elapsed = now - udpFlowValue->lastTime_.tv_sec;
		time_t timeout = 0;
		switch (udpFlowValue->state_) {
			case UdpFlowValue::Half: timeout = halfTimeout_; break;
			case UdpFlowValue::Full: timeout = fullTimeout_; break;
		}
		if (elapsed >= timeout) {
			GFlow::UdpFlowKey udpFlowKey = it.key();
			for (Managable* manager: managables_)
				manager->udpFlowDeleted(udpFlowKey, udpFlowValue);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GUdpFlowMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkIntervalSec_ != 0 && now - lastCheckClock_ >= checkIntervalSec_) {
		deleteOldFlowMaps(now);
		lastCheckClock_ = now;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GUdpHdr* udpHdr = packet->udpHdr_;
	if (udpHdr == nullptr) return;

	currentUdpFlowkey_.sip_ = ipHdr->sip();
	currentUdpFlowkey_.sport_ = udpHdr->sport();
	currentUdpFlowkey_.dip_ = ipHdr->dip();
	currentUdpFlowkey_.dport_ = udpHdr->dport();
	FlowMap::iterator it = flowMap_.find(currentUdpFlowkey_);

	currentRevUdpFlowKey_ = currentUdpFlowkey_.reverse();
	currentRevUdpFlowVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(currentRevUdpFlowKey_);
	if (rIt != flowMap_.end())
		currentRevUdpFlowVal_ = rIt.value();

	if (it == flowMap_.end()) {
		currentUdpFlowVal_ = UdpFlowValue::allocate(requestItems_.totalMemSize_);
		currentUdpFlowVal_->firstTime_ = currentUdpFlowVal_->lastTime_ = packet->ts_;

		if (currentRevUdpFlowVal_ == nullptr) {
			currentUdpFlowVal_->state_ = UdpFlowValue::Half;
			currentUdpFlowVal_->direction_ = ClientToServer;
		} else {
			currentUdpFlowVal_->state_ = UdpFlowValue::Full;
			currentUdpFlowVal_->direction_ = ServerToClient;
			currentRevUdpFlowVal_->state_ = UdpFlowValue::Full;
		}

		it = flowMap_.insert(currentUdpFlowkey_, currentUdpFlowVal_);
		currentPacket_ = packet;
		for (Managable* manager: managables_)
			manager->udpFlowCreated(currentUdpFlowkey_, currentUdpFlowVal_);
	} else {
		currentUdpFlowVal_ = it.value();
	}
	Q_ASSERT(currentUdpFlowVal_ != nullptr);
	currentUdpFlowVal_->lastTime_ = packet->ts_;
	currentUdpFlowVal_->packets_++;
	currentUdpFlowVal_->bytes_ += packet->buf_.size_;


	emit managed(packet);
}
