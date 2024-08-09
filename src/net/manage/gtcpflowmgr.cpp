#include "gtcpflowmgr.h"

// ----------------------------------------------------------------------------
// GTcpFlowMgr
// ----------------------------------------------------------------------------
bool GTcpFlowMgr::doOpen() {
	flowMap_.clear();
	return GPacketMgr::doOpen();
}

bool GTcpFlowMgr::doClose() {
	while (true) {
		if (flowMap_.count() == 0) break;
		FlowMap::iterator oldestIt = flowMap_.begin();
		GFlow::TcpFlowKey oldestTcpFlowKey = oldestIt.key();
		TcpFlowValue* oldestTcpFlowValue = oldestIt.value();
		FlowMap::iterator it = oldestIt + 1;
		for (; it != flowMap_.end(); it++) {
			TcpFlowValue* tcpFlowValue = it.value();
			if (oldestTcpFlowValue->lastTime_.tv_sec > tcpFlowValue->lastTime_.tv_sec) {
				oldestIt = it;
				oldestTcpFlowKey = it.key();
				oldestTcpFlowValue = it.value();
			}
		}
		for (Managable* manager: managables_)
			manager->tcpFlowDeleted(oldestTcpFlowKey, oldestTcpFlowValue);
		flowMap_.erase(oldestIt);
	}
	Q_ASSERT(flowMap_.count() == 0);
	return GPacketMgr::doClose();
}

void GTcpFlowMgr::deleteOldFlowMaps(time_t now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		TcpFlowValue* tcpFlowValue = it.value();
		time_t elapsed = now - tcpFlowValue->lastTime_.tv_sec;
		time_t timeout = 0;
		switch (tcpFlowValue->state_) {
			case TcpFlowValue::Half: timeout = halfTimeout_; break;
			case TcpFlowValue::Full: timeout = fullTimeout_; break;
			case TcpFlowValue::Rst: timeout = rstTimeout_; break;
			case TcpFlowValue::Fin: timeout = finTimeout_; break;
		}
		if (elapsed >= timeout) {
			GFlow::TcpFlowKey tcpFlowKey = it.key();
			for (Managable* manager: managables_)
				manager->tcpFlowDeleted(tcpFlowKey, tcpFlowValue);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GTcpFlowMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkIntervalSec_ != 0 && now - lastCheckClock_ >= checkIntervalSec_) {
		deleteOldFlowMaps(now);
		lastCheckClock_ = now;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) return;

	currentTcpFlowKey_.sip_ = ipHdr->sip();
	currentTcpFlowKey_.sport_ = tcpHdr->sport();
	currentTcpFlowKey_.dip_ = ipHdr->dip();
	currentTcpFlowKey_.dport_ = tcpHdr->dport();
	FlowMap::iterator it = flowMap_.find(currentTcpFlowKey_);

	currentRevTcpFlowKey_ = currentTcpFlowKey_.reverse();
	currentRevTcpFlowVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(currentRevTcpFlowKey_);
	if (rIt != flowMap_.end())
		currentRevTcpFlowVal_ = rIt.value();

	if (it == flowMap_.end()) {
		currentTcpFlowVal_ = TcpFlowValue::allocate(requestItems_.totalMemSize_);
		currentTcpFlowVal_->firstTime_ = currentTcpFlowVal_->lastTime_ = packet->ts_;

		if (currentRevTcpFlowVal_ == nullptr) {
			currentTcpFlowVal_->state_ = TcpFlowValue::Half;
			currentTcpFlowVal_->direction_ = ClientToServer;
		} else {
			currentTcpFlowVal_->state_ = TcpFlowValue::Full;
			currentTcpFlowVal_->direction_ = ServerToClient;
			currentRevTcpFlowVal_->state_ = TcpFlowValue::Full;
		}

		it = flowMap_.insert(currentTcpFlowKey_, currentTcpFlowVal_);
		currentPacket_ = packet;
		for (Managable* manager: managables_)
			manager->tcpFlowCreated(currentTcpFlowKey_, currentTcpFlowVal_);
	} else {
		currentTcpFlowVal_ = it.value();
	}
	Q_ASSERT(currentTcpFlowVal_ != nullptr);
	currentTcpFlowVal_->lastTime_ = packet->ts_;
	currentTcpFlowVal_->packets_++;
	currentTcpFlowVal_->bytes_ += packet->buf_.size_;

	if ((tcpHdr->flags() & (GTcpHdr::Rst | GTcpHdr::Fin)) != 0) {
		TcpFlowValue::State state = (tcpHdr->flags() & GTcpHdr::Rst) ? TcpFlowValue::Rst : TcpFlowValue::Fin;
		currentTcpFlowVal_->state_ = state;
		if (currentRevTcpFlowVal_ != nullptr) {
			currentRevTcpFlowVal_->state_ = state;
		}
	}

	emit managed(packet);
}
