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
			GPacketMgr::Value* value = it.value();
			manager->tcpFlowDeleted(tcpFlowKey, value);
		}
	}
	flowMap_.clear();
	return GPacketMgr::doClose();
}

void GTcpFlowMgr::deleteOldFlowMaps(long now) {
	FlowMap::iterator it = flowMap_.begin();
	while (it != flowMap_.end()) {
		GPacketMgr::Value* value = it.value();
		long elapsed = now - value->ts_.tv_sec;
		long timeout = 0;
		switch (value->state_) {
			case GPacketMgr::Value::Half: timeout = halfTimeout_; break;
			case GPacketMgr::Value::Full: timeout = fullTimeout_; break;
			case GPacketMgr::Value::Rst: timeout = rstTimeout_; break;
			case GPacketMgr::Value::Fin: timeout = finTimeout_; break;
		}
		if (elapsed >= timeout) {
			GFlow::TcpFlowKey tcpFlowKey = it.key();
			for (Managable* manager: managables_)
				manager->tcpFlowDeleted(tcpFlowKey, value);
			it = flowMap_.erase(it);
			continue;
		}
		it++;
	}
}

void GTcpFlowMgr::manage(GPacket* packet) {
	long now = packet->ts_.tv_sec;
	if (checkIntervalSec_ != 0 && now - lastCheckTick_ >= checkIntervalSec_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
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
	currentRevVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(currentRevTcpFlowKey_);
	if (rIt != flowMap_.end())
		currentRevVal_ = rIt.value();

	if (it == flowMap_.end()) {
		currentVal_ = GPacketMgr::Value::allocate(GPacketMgr::Value::Half, requestItems_.totalMemSize_);
		it = flowMap_.insert(currentTcpFlowKey_, currentVal_);
		for (Managable* manager: managables_)
			manager->tcpFlowDetected(currentTcpFlowKey_, currentVal_);

		if (currentRevVal_ != nullptr) {
			currentVal_->state_ = GPacketMgr::Value::Full;
			currentRevVal_->state_ = GPacketMgr::Value::Full;
		}
	} else {
		currentVal_ = it.value();
	}
	Q_ASSERT(currentVal_ != nullptr);
	currentVal_->ts_ = packet->ts_;

	if ((tcpHdr->flags() & (GTcpHdr::Rst | GTcpHdr::Fin)) != 0) {
		GPacketMgr::Value::State state = (tcpHdr->flags() & GTcpHdr::Rst) ? GPacketMgr::Value::Rst : GPacketMgr::Value::Fin;
		currentVal_->state_ = state;
		if (currentRevVal_ != nullptr) {
			currentRevVal_->state_ = state;
		}
	}

	emit managed(packet);
}
