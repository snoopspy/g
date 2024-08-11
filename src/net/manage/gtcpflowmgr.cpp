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
		FlowMap::iterator it = oldestIt;
		it++;
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
			case TcpFlowValue::ForwardFin: timeout = forwardFinTimeout_; break;
			case TcpFlowValue::BackwardFin: timeout = backwardFinTimeout_; break;
			case TcpFlowValue::FullFin: timeout = fullFinTimeout_; break;
			case TcpFlowValue::ForwardRst: timeout = forwardRstTimeout_; break;
			case TcpFlowValue::BackwardRst: timeout = forwardRstTimeout_; break;
			case TcpFlowValue::FullRst: timeout = fullRstTimeout_; break;
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

	uint8_t flag = tcpHdr->flags();
	if ((flag & GTcpHdr::Fin) != 0) {
		TcpFlowValue::State oldState = currentTcpFlowVal_->state_;
		switch (oldState) {
			case TcpFlowValue::Half:
				currentTcpFlowVal_->state_ = TcpFlowValue::ForwardFin;
				break;
			case TcpFlowValue::Full:
				currentTcpFlowVal_->state_ = TcpFlowValue::ForwardFin;
				break;
			case TcpFlowValue::ForwardFin: // do not modify
				break;
			case TcpFlowValue::BackwardFin: // full !!!
				currentTcpFlowVal_->state_= TcpFlowValue::FullFin;
				break;
			case TcpFlowValue::FullFin: // do not modify
				break;
			case TcpFlowValue::ForwardRst: // do not modify
				qWarning() << "invalid state ForwardRst";
				break;
			case TcpFlowValue::BackwardRst:
				currentTcpFlowVal_->state_ = TcpFlowValue::FullFin;
				break;
			case TcpFlowValue::FullRst: // do not modify
				qWarning() << "invalid state FullRst";
				break;
		}

		if (currentRevTcpFlowVal_ != nullptr) {
			TcpFlowValue::State revOldState = currentRevTcpFlowVal_->state_;
			switch (revOldState) {
				case TcpFlowValue::Half:
					currentRevTcpFlowVal_->state_ = TcpFlowValue::BackwardFin;
					break;
				case TcpFlowValue::Full:
					currentRevTcpFlowVal_->state_ = TcpFlowValue::BackwardFin;
					break;
				case TcpFlowValue::ForwardFin: // full !!!
					currentRevTcpFlowVal_->state_ = TcpFlowValue::FullFin;
					break;
				case TcpFlowValue::BackwardFin: // do not modify
					break;
				case TcpFlowValue::FullFin: // do not modify
					break;
				case TcpFlowValue::ForwardRst: // do not modify
					break;
				case TcpFlowValue::BackwardRst: // do not modify
					break;
				case TcpFlowValue::FullRst: // do not modify
					break;
			}
		}
	} else if ((flag & GTcpHdr::Rst) != 0) {
		TcpFlowValue::State oldState = currentTcpFlowVal_->state_;
		switch (oldState) {
			case TcpFlowValue::Half:
				currentTcpFlowVal_->state_ = TcpFlowValue::ForwardRst;
				break;
			case TcpFlowValue::Full:
				currentTcpFlowVal_->state_ = TcpFlowValue::ForwardRst;
				break;
			case TcpFlowValue::ForwardFin:
				currentTcpFlowVal_->state_ = TcpFlowValue::ForwardRst;
				break;
			case TcpFlowValue::BackwardFin:
				currentTcpFlowVal_->state_ = TcpFlowValue::ForwardRst;
				break;
			case TcpFlowValue::FullFin:
				currentTcpFlowVal_->state_ = TcpFlowValue::ForwardRst;
				break;
			case TcpFlowValue::ForwardRst: // do not modify
				break;
			case TcpFlowValue::BackwardRst: // full !!!
				currentTcpFlowVal_->state_ = TcpFlowValue::FullRst;
				break;
			case TcpFlowValue::FullRst: // do not modify
				break;
		}
		if (currentRevTcpFlowVal_ != nullptr) {
			TcpFlowValue::State revOldState = currentRevTcpFlowVal_->state_;
			switch(revOldState) {
				case TcpFlowValue::Half:
					currentRevTcpFlowVal_->state_ = TcpFlowValue::BackwardRst;
					break;
				case TcpFlowValue::Full:
					currentRevTcpFlowVal_->state_ = TcpFlowValue::BackwardRst;
					break;
				case TcpFlowValue::ForwardFin:
					currentRevTcpFlowVal_->state_ = TcpFlowValue::FullRst;
					break;
				case TcpFlowValue::BackwardFin:
					currentRevTcpFlowVal_->state_ = TcpFlowValue::BackwardRst;
					break;
				case TcpFlowValue::FullFin:
					currentRevTcpFlowVal_->state_ = TcpFlowValue::FullRst;
					break;
				case TcpFlowValue::ForwardRst: // full !!!
					currentRevTcpFlowVal_->state_ = TcpFlowValue::FullRst;
					break;
				case TcpFlowValue::BackwardRst: // do not modiry
					break;
				case TcpFlowValue::FullRst: // do not modify
					break;
			}
		}
	}

	emit managed(packet);
}
