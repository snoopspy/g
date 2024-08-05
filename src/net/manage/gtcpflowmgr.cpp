#include "gtcpflowmgr.h"

// ----------------------------------------------------------------------------
// GTcpFlowMgr
// ----------------------------------------------------------------------------
bool GTcpFlowMgr::doOpen() {
	flowMap_.clear();
	return GPacketMgr::doOpen();
}

bool GTcpFlowMgr::doClose() {
	for (Managable* manager: managables_) {
		for (FlowMap::iterator it = flowMap_.begin(); it != flowMap_.end(); it++) {
			GFlow::TcpFlowKey tcpFlowKey = it.key();
			TcpFlowValue* tcpFlowValue = it.value();
			manager->tcpFlowDeleted(tcpFlowKey, tcpFlowValue);
		}
	}
	flowMap_.clear();
	return GPacketMgr::doClose();
}

void GTcpFlowMgr::deleteOldFlowMaps(time_t now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		TcpFlowValue* tcpFlowValue = it.value();
		time_t elapsed = now - tcpFlowValue->lastTime_;
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
		currentTcpFlowVal_->firstTime_ = currentTcpFlowVal_->lastTime_ = packet->ts_.tv_sec;

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
	currentTcpFlowVal_->lastTime_ = packet->ts_.tv_sec;
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
