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
	if (checkInterval_ != 0 && now - lastCheckTick_ >= checkInterval_) {
		deleteOldFlowMaps(now);
		lastCheckTick_ = now;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) return;

	tcpFlowKey_.sip_ = ipHdr->sip();
	tcpFlowKey_.sport_ = tcpHdr->sport();
	tcpFlowKey_.dip_ = ipHdr->dip();
	tcpFlowKey_.dport_ = tcpHdr->dport();
	FlowMap::iterator it = flowMap_.find(tcpFlowKey_);

	rTcpFlowKey_ = tcpFlowKey_.reverse();
	rVal_ = nullptr;
	FlowMap::iterator rIt = flowMap_.find(rTcpFlowKey_);
	if (rIt != flowMap_.end())
		rVal_ = rIt.value();

	if (it == flowMap_.end()) {
		val_ = GPacketMgr::Value::allocate(GPacketMgr::Value::Half, requestItems_.totalMemSize_);
		it = flowMap_.insert(tcpFlowKey_, val_);
		for (Managable* manager: managables_)
			manager->tcpFlowDetected(tcpFlowKey_, val_);

		if (rVal_ != nullptr) {
			val_->state_ = GPacketMgr::Value::Full;
			rVal_->state_ = GPacketMgr::Value::Full;
		}
	} else {
		val_ = it.value();
	}
	Q_ASSERT(val_ != nullptr);
	val_->ts_ = packet->ts_;

	if ((tcpHdr->flags() & (GTcpHdr::Rst | GTcpHdr::Fin)) != 0) {
		GPacketMgr::Value::State state = (tcpHdr->flags() & GTcpHdr::Rst) ? GPacketMgr::Value::Rst : GPacketMgr::Value::Fin;
		val_->state_ = state;
		if (rVal_ != nullptr) {
			rVal_->state_ = state;
		}
	}

	emit managed(packet);
}
