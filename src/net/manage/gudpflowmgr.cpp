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
				manager->udpFlowCreated(udpFlowKey, value);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GUdpFlowMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkInterval_ != 0 && now - lastCheckTick_ >= checkInterval_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GUdpHdr* udpHdr = packet->udpHdr_;
	if (udpHdr == nullptr) return;

	udpFlowkey_.sip_ = ipHdr->sip();
	udpFlowkey_.sport_ = udpHdr->sport();
	udpFlowkey_.dip_ = ipHdr->dip();
	udpFlowkey_.dport_ = udpHdr->dport();
	FlowMap::iterator it = flowMap_.find(udpFlowkey_);

	rUdpFlowKey_ = udpFlowkey_.reverse();
	rVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(rUdpFlowKey_);
	if (rIt != flowMap_.end())
		rVal_ = rIt.value();

	if (it == flowMap_.end()) {
		val_ = GPacketMgr::Value::allocate(GPacketMgr::Value::Half, requestItems_.totalMemSize_);
		it = flowMap_.insert(udpFlowkey_, val_);
		for (Managable* manager: managables_)
			manager->udpFlowCreated(udpFlowkey_, val_);

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
