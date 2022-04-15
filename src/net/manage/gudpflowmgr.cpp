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
			GPacketMgr::Value* value = it.value();
			manager->udpFlowDeleted(udpFlowKey, value);
		}
	}
	flowMap_.clear();
	return GPacketMgr::doClose();
}

void GUdpFlowMgr::deleteOldFlowMaps(long now) {
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
			GFlow::UdpFlowKey udpFlowKey = it.key();
			for (Managable* manager: managables_)
				manager->udpFlowDetected(udpFlowKey, value);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GUdpFlowMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkIntervalSec_ != 0 && now - lastCheckTick_ >= checkIntervalSec_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
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
	currentRevVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(currentRevUdpFlowKey_);
	if (rIt != flowMap_.end())
		currentRevVal_ = rIt.value();

	if (it == flowMap_.end()) {
		currentVal_ = GPacketMgr::Value::allocate(GPacketMgr::Value::Half, requestItems_.totalMemSize_);
		it = flowMap_.insert(currentUdpFlowkey_, currentVal_);
		for (Managable* manager: managables_)
			manager->udpFlowDetected(currentUdpFlowkey_, currentVal_);

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
