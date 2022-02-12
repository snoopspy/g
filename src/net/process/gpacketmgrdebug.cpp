#include "gpacketmgrdebug.h"

// ----------------------------------------------------------------------------
// GPacketMgrDebug
// ----------------------------------------------------------------------------
bool GPacketMgrDebug::doOpen() {
	if (ipFlowMgr_ != nullptr) {
		ipFlowOffset_ = ipFlowMgr_->requestItems_.request("GFlowMgrTest_ip", sizeof(FlowItem));
		ipFlowMgr_->managables_.insert(this);
	}

	if (tcpFlowMgr_ != nullptr) {
		tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request("GFlowMgrTest_tcp", sizeof(FlowItem));
		tcpFlowMgr_->managables_.insert(this);
	}

	if (udpFlowMgr_ != nullptr) {
		udpFlowOffset_ = udpFlowMgr_->requestItems_.request("GFlowMgrTest_udp", sizeof(FlowItem));
		udpFlowMgr_->managables_.insert(this);
	}
	return true;
}

bool GPacketMgrDebug::doClose() {
	return true;
}

void GPacketMgrDebug::ipFlowCreated(GFlow::IpFlowKey* key, GPktMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("_ipFlowCreated %1>%2").arg(QString(key->sip_), QString(key->dip_));
	FlowItem* flowItem = PFlowItem(value->mem(ipFlowOffset_));
	new (flowItem) FlowItem;
}

void GPacketMgrDebug::ipFlowDeleted(GFlow::IpFlowKey* key, GPktMgr::Value* value) {
	(void)value;
	if (!enabled_) return;
	qDebug() << QString("_ipFlowDeleted %1>%2").arg(QString(key->sip_), QString(key->dip_));
}

void GPacketMgrDebug::tcpFlowCreated(GFlow::TcpFlowKey* key, GPktMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("_tcpFlowCreated %1:%2>%3:%4").arg(QString(key->sip_), QString::number(key->sport_), QString(key->dip_), QString::number(key->dport_));
	FlowItem* flowItem = PFlowItem(value->mem(tcpFlowOffset_));
	new (flowItem) FlowItem;
}

void GPacketMgrDebug::tcpFlowDeleted(GFlow::TcpFlowKey* key, GPktMgr::Value* value) {
	(void)value;
	if (!enabled_) return;
	qDebug() << QString("_tcpFlowDeleted %1:%2>%3:%4").arg(QString(key->sip_), QString::number(key->sport_), QString(key->dip_), QString::number(key->dport_));
}

void GPacketMgrDebug::udpFlowCreated(GFlow::UdpFlowKey* key, GPktMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("_udpFlowCreated %1:%2>%3:%4").arg(QString(key->sip_), QString::number(key->sport_), QString(key->dip_), QString::number(key->dport_));
	FlowItem* flowItem = PFlowItem(value->mem(ipFlowOffset_));
	new (flowItem) FlowItem;
}

void GPacketMgrDebug::udpFlowDeleted(GFlow::UdpFlowKey* key, GPktMgr::Value* value) {
	(void)value;
	if (!enabled_) return;
	qDebug() << QString("_udpFlowDeleted %1:%2>%3:%4").arg(QString(key->sip_), QString::number(key->sport_), QString(key->dip_), QString::number(key->dport_));
}

void GPacketMgrDebug::debug(GPacket* packet) {
	if (!enabled_) return;

	if (packet->ipHdr_ != nullptr) {
		if (ipFlowMgr_ != nullptr) {
			Q_ASSERT(ipFlowMgr_->val_ != nullptr);
			FlowItem* flowItem = PFlowItem(ipFlowMgr_->val_->mem(ipFlowOffset_));
			flowItem->packets++;
			flowItem->bytes += packet->buf_.size_;
			GFlow::IpFlowKey* key = &ipFlowMgr_->key_;
			qDebug() << QString("ip size=%1 packets=%2 bytes=%3 %4>%5")
				.arg(packet->buf_.size_).arg(flowItem->packets).arg(flowItem->bytes)
				.arg(QString(key->sip_)).arg(QString(key->dip_)); // gilgil temp 2016.10.10
		}

		if (packet->tcpHdr_ != nullptr) {
			if (tcpFlowMgr_ != nullptr) {
				Q_ASSERT(tcpFlowMgr_->val_ != nullptr);
				FlowItem* flowItem = PFlowItem(tcpFlowMgr_->val_->mem(tcpFlowOffset_));
				flowItem->packets++;
				flowItem->bytes += packet->buf_.size_;
				GFlow::TcpFlowKey* key = &tcpFlowMgr_->key_;
				qDebug() << QString("tcp size=%1 packets=%2 bytes=%3 %4:%5>%6:%7")
					.arg(packet->buf_.size_).arg(flowItem->packets).arg(flowItem->bytes)
					.arg(QString(key->sip_)).arg(key->sport_).arg(QString(key->dip_)).arg(key->dport_); // gilgil temp 2016.10.10
			}
		}

		if (packet->udpHdr_ != nullptr) {
			if (udpFlowMgr_ != nullptr) {
				Q_ASSERT(udpFlowMgr_->val_ != nullptr);
				FlowItem* flowItem = PFlowItem(udpFlowMgr_->val_->mem(udpFlowOffset_));
				flowItem->packets++;
				flowItem->bytes += packet->buf_.size_;
				GFlow::UdpFlowKey* key = &udpFlowMgr_->key_;
				qDebug() << QString("udp size=%1 packets=%2 bytes=%3 %4:%5>%6:%7")
					.arg(packet->buf_.size_).arg(flowItem->packets).arg(flowItem->bytes)
					.arg(QString(key->sip_)).arg(key->sport_).arg(QString(key->dip_)).arg(key->dport_); // gilgil temp 2016.10.10
			}
		}
	}

	emit debugged(packet);
}
