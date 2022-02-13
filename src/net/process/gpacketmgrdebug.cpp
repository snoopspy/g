#include "gpacketmgrdebug.h"

// ----------------------------------------------------------------------------
// GPacketMgrDebug
// ----------------------------------------------------------------------------
bool GPacketMgrDebug::doOpen() {
	if (hostMgr_ != nullptr) {
		hostOffset_ = hostMgr_->requestItems_.request("GPacketMgrDebug-host", sizeof(FlowItem));
		hostMgr_->managables_.insert(this);
	}

	if (ipFlowMgr_ != nullptr) {
		ipFlowOffset_ = ipFlowMgr_->requestItems_.request("GPacketMgrDebug-ipFlow", sizeof(FlowItem));
		ipFlowMgr_->managables_.insert(this);
	}

	if (tcpFlowMgr_ != nullptr) {
		tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request("GPacketMgrDebug-tcpFlow", sizeof(FlowItem));
		tcpFlowMgr_->managables_.insert(this);
	}

	if (udpFlowMgr_ != nullptr) {
		udpFlowOffset_ = udpFlowMgr_->requestItems_.request("GPacketMgrDebug-udpFlow", sizeof(FlowItem));
		udpFlowMgr_->managables_.insert(this);
	}
	return true;
}

bool GPacketMgrDebug::doClose() {
	return true;
}

void GPacketMgrDebug::hostCreated(GMac mac, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("hostCreated %1").arg(QString(mac));
	FlowItem* flowItem = PFlowItem(value->mem(hostOffset_));
	new (flowItem) FlowItem;
}

void GPacketMgrDebug::hostDeleted(GMac mac, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("hostDeleted %1").arg(QString(mac));
	FlowItem* flowItem = PFlowItem(value->mem(hostOffset_));
	flowItem->~FlowItem();
}

void GPacketMgrDebug::ipFlowCreated(GFlow::IpFlowKey ipFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("ipFlowCreated %1>%2").arg(QString(ipFlowKey.sip_), QString(ipFlowKey.dip_));
	FlowItem* flowItem = PFlowItem(value->mem(ipFlowOffset_));
	new (flowItem) FlowItem;
}

void GPacketMgrDebug::ipFlowDeleted(GFlow::IpFlowKey ipFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("ipFlowDeleted %1>%2").arg(QString(ipFlowKey.sip_), QString(ipFlowKey.dip_));
	FlowItem* flowItem = PFlowItem(value->mem(ipFlowOffset_));
	flowItem->~FlowItem();
}

void GPacketMgrDebug::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("tcpFlowCreated %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_), QString::number(tcpFlowKey.sport_), QString(tcpFlowKey.dip_), QString::number(tcpFlowKey.dport_));
	FlowItem* flowItem = PFlowItem(value->mem(tcpFlowOffset_));
	new (flowItem) FlowItem;
}

void GPacketMgrDebug::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("tcpFlowDeleted %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_), QString::number(tcpFlowKey.sport_), QString(tcpFlowKey.dip_), QString::number(tcpFlowKey.dport_));
	FlowItem* flowItem = PFlowItem(value->mem(tcpFlowOffset_));
	flowItem->~FlowItem();
}

void GPacketMgrDebug::udpFlowCreated(GFlow::UdpFlowKey udpFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("udpFlowCreated %1:%2>%3:%4").arg(QString(udpFlowKey.sip_), QString::number(udpFlowKey.sport_), QString(udpFlowKey.dip_), QString::number(udpFlowKey.dport_));
	FlowItem* flowItem = PFlowItem(value->mem(ipFlowOffset_));
	new (flowItem) FlowItem;
}

void GPacketMgrDebug::udpFlowDeleted(GFlow::UdpFlowKey udpFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("udpFlowDeleted %1:%2>%3:%4").arg(QString(udpFlowKey.sip_), QString::number(udpFlowKey.sport_), QString(udpFlowKey.dip_), QString::number(udpFlowKey.dport_));
	FlowItem* flowItem = PFlowItem(value->mem(ipFlowOffset_));
	flowItem->~FlowItem();
}

void GPacketMgrDebug::debug(GPacket* packet) {
	if (!enabled_) return;

	if (packet->ipHdr_ != nullptr) {
		if (ipFlowMgr_ != nullptr) {
			Q_ASSERT(ipFlowMgr_->val_ != nullptr);
			FlowItem* flowItem = PFlowItem(ipFlowMgr_->val_->mem(ipFlowOffset_));
			flowItem->packets++;
			flowItem->bytes += packet->buf_.size_;
			GFlow::IpFlowKey* ipFlowKey = &ipFlowMgr_->ipFlowKey_;
			qDebug() << QString("ip size=%1 packets=%2 bytes=%3 %4>%5")
				.arg(packet->buf_.size_).arg(flowItem->packets).arg(flowItem->bytes)
				.arg(QString(ipFlowKey->sip_)).arg(QString(ipFlowKey->dip_)); // gilgil temp 2016.10.10
		}

		if (packet->tcpHdr_ != nullptr) {
			if (tcpFlowMgr_ != nullptr) {
				Q_ASSERT(tcpFlowMgr_->val_ != nullptr);
				FlowItem* flowItem = PFlowItem(tcpFlowMgr_->val_->mem(tcpFlowOffset_));
				flowItem->packets++;
				flowItem->bytes += packet->buf_.size_;
				GFlow::TcpFlowKey* tcpFlowKey = &tcpFlowMgr_->tcpFlowKey_;
				qDebug() << QString("tcp size=%1 packets=%2 bytes=%3 %4:%5>%6:%7")
					.arg(packet->buf_.size_).arg(flowItem->packets).arg(flowItem->bytes)
					.arg(QString(tcpFlowKey->sip_)).arg(tcpFlowKey->sport_).arg(QString(tcpFlowKey->dip_)).arg(tcpFlowKey->dport_); // gilgil temp 2016.10.10
			}
		}

		if (packet->udpHdr_ != nullptr) {
			if (udpFlowMgr_ != nullptr) {
				Q_ASSERT(udpFlowMgr_->val_ != nullptr);
				FlowItem* flowItem = PFlowItem(udpFlowMgr_->val_->mem(udpFlowOffset_));
				flowItem->packets++;
				flowItem->bytes += packet->buf_.size_;
				GFlow::UdpFlowKey* udpFlowKey = &udpFlowMgr_->udpFlowkey_;
				qDebug() << QString("udp size=%1 packets=%2 bytes=%3 %4:%5>%6:%7")
					.arg(packet->buf_.size_).arg(flowItem->packets).arg(flowItem->bytes)
					.arg(QString(udpFlowKey->sip_)).arg(udpFlowKey->sport_).arg(QString(udpFlowKey->dip_)).arg(udpFlowKey->dport_); // gilgil temp 2016.10.10
			}
		}
	}

	emit debugged(packet);
}
