#include "gtcpflowmgr.h"

// ----------------------------------------------------------------------------
// GTcpFlowMgr
// ----------------------------------------------------------------------------
bool GTcpFlowMgr::doOpen() {
	flowMap_.clear();
	return GMgr::doOpen();
}

bool GTcpFlowMgr::doClose() {
	for (Managable* manager: managables_) {
		for (FlowMap::iterator it = flowMap_.begin(); it != flowMap_.end(); it++) {
			GFlow::TcpFlowKey key = it.key();
			GMgr::Value* value = it.value();
			manager->tcpFlowDeleted(&key, value);
		}
	}
	flowMap_.clear();
	return GMgr::doClose();
}

void GTcpFlowMgr::deleteOldFlowMaps(long now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		GMgr::Value* value = it.value();
		long elapsed = now - value->ts_.tv_sec;
		long timeout = 0;
		switch (value->state_) {
			case GMgr::Value::Half: timeout = halfTimeout_; break;
			case GMgr::Value::Full: timeout = fullTimeout_; break;
			case GMgr::Value::Rst: timeout = rstTimeout_; break;
			case GMgr::Value::Fin: timeout = finTimeout_; break;
		}
		if (elapsed >= timeout) {
			GFlow::TcpFlowKey* key = const_cast<GFlow::TcpFlowKey*>(&it.key());
			for (Managable* manager: managables_)
				manager->tcpFlowDeleted(key, value);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GTcpFlowMgr::process(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkInterval_ != 0 && now - lastCheckTick_ >= checkInterval_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) return;

	key_.sip_ = ipHdr->sip();
	key_.sport_ = tcpHdr->sport();
	key_.dip_ = ipHdr->dip();
	key_.dport_ = tcpHdr->dport();
	FlowMap::iterator it = flowMap_.find(key_);

	rKey_ = key_.reverse();
	rVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(rKey_);
	if (rIt != flowMap_.end())
		rVal_ = rIt.value();

	if (it == flowMap_.end()) {
		val_ = GMgr::Value::allocate(GMgr::Value::Half, requestItems_.totalMemSize_);
		it = flowMap_.insert(key_, val_);
		for (Managable* manager: managables_)
			manager->tcpFlowCreated(&key_, val_);

		if (rVal_ != nullptr) {
			val_->state_ = GMgr::Value::Full;
			rVal_->state_ = GMgr::Value::Full;
		}
	} else {
		val_ = it.value();
	}
	Q_ASSERT(val_ != nullptr);
	val_->ts_ = packet->ts_;

	if ((tcpHdr->flags() & (GTcpHdr::Rst | GTcpHdr::Fin)) != 0) {
		GMgr::Value::State state = (tcpHdr->flags() & GTcpHdr::Rst) ? GMgr::Value::Rst : GMgr::Value::Fin;
		val_->state_ = state;
		if (rVal_ != nullptr) {
			rVal_->state_ = state;
		}
	}

	emit processed(packet);
}
