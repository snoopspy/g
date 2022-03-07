#include "gpacketmgrdebug.h"

// ----------------------------------------------------------------------------
// GPacketMgrDebug
// ----------------------------------------------------------------------------
bool GPacketMgrDebug::doOpen() {
	if (hostMgr_ != nullptr) {
		hostOffset_ = hostMgr_->requestItems_.request("GPacketMgrDebug-host", sizeof(Item));
		hostMgr_->managables_.insert(this);
	}

	if (ipFlowMgr_ != nullptr) {
		ipFlowOffset_ = ipFlowMgr_->requestItems_.request("GPacketMgrDebug-ipFlow", sizeof(Item));
		ipFlowMgr_->managables_.insert(this);
	}

	if (tcpFlowMgr_ != nullptr) {
		tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request("GPacketMgrDebug-tcpFlow", sizeof(Item));
		tcpFlowMgr_->managables_.insert(this);
	}

	if (udpFlowMgr_ != nullptr) {
		udpFlowOffset_ = udpFlowMgr_->requestItems_.request("GPacketMgrDebug-udpFlow", sizeof(Item));
		udpFlowMgr_->managables_.insert(this);
	}
	return true;
}

bool GPacketMgrDebug::doClose() {
	return true;
}

void GPacketMgrDebug::hostDetected(GMac mac, GHostMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("hostDetected %1 %2").arg(QString(mac), QString(value->ip_));
	Item* item = PItem(value->mem(hostOffset_));
	new (item) Item;
}

void GPacketMgrDebug::hostDeleted(GMac mac, GHostMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("hostDeleted %1 %2").arg(QString(mac), QString(value->ip_));
	Item* item = PItem(value->mem(hostOffset_));
	item->~Item();
}

void GPacketMgrDebug::ipFlowDetected(GFlow::IpFlowKey ipFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("ipFlowDetected %1>%2").arg(QString(ipFlowKey.sip_), QString(ipFlowKey.dip_));
	Item* item = PItem(value->mem(ipFlowOffset_));
	new (item) Item;
}

void GPacketMgrDebug::ipFlowDeleted(GFlow::IpFlowKey ipFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("ipFlowDeleted %1>%2").arg(QString(ipFlowKey.sip_), QString(ipFlowKey.dip_));
	Item* item = PItem(value->mem(ipFlowOffset_));
	item->~Item();
}

void GPacketMgrDebug::tcpFlowDetected(GFlow::TcpFlowKey tcpFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("tcpFlowDetected %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_), QString::number(tcpFlowKey.sport_), QString(tcpFlowKey.dip_), QString::number(tcpFlowKey.dport_));
	Item* item = PItem(value->mem(tcpFlowOffset_));
	new (item) Item;
}

void GPacketMgrDebug::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("tcpFlowDeleted %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_), QString::number(tcpFlowKey.sport_), QString(tcpFlowKey.dip_), QString::number(tcpFlowKey.dport_));
	Item* item = PItem(value->mem(tcpFlowOffset_));
	item->~Item();
}

void GPacketMgrDebug::udpFlowDetected(GFlow::UdpFlowKey udpFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("udpFlowDetected %1:%2>%3:%4").arg(QString(udpFlowKey.sip_), QString::number(udpFlowKey.sport_), QString(udpFlowKey.dip_), QString::number(udpFlowKey.dport_));
	Item* item = PItem(value->mem(ipFlowOffset_));
	new (item) Item;
}

void GPacketMgrDebug::udpFlowDeleted(GFlow::UdpFlowKey udpFlowKey, GPacketMgr::Value* value) {
	if (!enabled_) return;
	qDebug() << QString("udpFlowDeleted %1:%2>%3:%4").arg(QString(udpFlowKey.sip_), QString::number(udpFlowKey.sport_), QString(udpFlowKey.dip_), QString::number(udpFlowKey.dport_));
	Item* item = PItem(value->mem(ipFlowOffset_));
	item->~Item();
}

void GPacketMgrDebug::debug(GPacket* packet) {
	if (!enabled_) return;

	if (packet->ethHdr_ != nullptr) {
		if (hostMgr_ != nullptr) {
			Q_ASSERT(hostMgr_->val_ != nullptr);
			Item* item = PItem(hostMgr_->val_->mem(hostOffset_));
			item->packets++;
			item->bytes += packet->buf_.size_;
			GMac mac = hostMgr_->mac_;
			qDebug() << QString("eth size=%1 packets=%2 bytes=%3 %4")
				.arg(packet->buf_.size_).arg(item->packets).arg(item->bytes)
				.arg(QString(mac));
		}
	}

	if (packet->ipHdr_ != nullptr) {
		if (ipFlowMgr_ != nullptr) {
			Q_ASSERT(ipFlowMgr_->val_ != nullptr);
			Item* item = PItem(ipFlowMgr_->val_->mem(ipFlowOffset_));
			item->packets++;
			item->bytes += packet->buf_.size_;
			GFlow::IpFlowKey* ipFlowKey = &ipFlowMgr_->ipFlowKey_;
			qDebug() << QString("ip size=%1 packets=%2 bytes=%3 %4>%5")
				.arg(packet->buf_.size_).arg(item->packets).arg(item->bytes)
				.arg(QString(ipFlowKey->sip_)).arg(QString(ipFlowKey->dip_));
		}

		if (packet->tcpHdr_ != nullptr) {
			if (tcpFlowMgr_ != nullptr) {
				Q_ASSERT(tcpFlowMgr_->val_ != nullptr);
				Item* item = PItem(tcpFlowMgr_->val_->mem(tcpFlowOffset_));
				item->packets++;
				item->bytes += packet->buf_.size_;
				GFlow::TcpFlowKey* tcpFlowKey = &tcpFlowMgr_->tcpFlowKey_;
				qDebug() << QString("tcp size=%1 packets=%2 bytes=%3 %4:%5>%6:%7")
					.arg(packet->buf_.size_).arg(item->packets).arg(item->bytes)
					.arg(QString(tcpFlowKey->sip_)).arg(tcpFlowKey->sport_).arg(QString(tcpFlowKey->dip_)).arg(tcpFlowKey->dport_);
			}
		}

		if (packet->udpHdr_ != nullptr) {
			if (udpFlowMgr_ != nullptr) {
				Q_ASSERT(udpFlowMgr_->val_ != nullptr);
				Item* item = PItem(udpFlowMgr_->val_->mem(udpFlowOffset_));
				item->packets++;
				item->bytes += packet->buf_.size_;
				GFlow::UdpFlowKey* udpFlowKey = &udpFlowMgr_->udpFlowkey_;
				qDebug() << QString("udp size=%1 packets=%2 bytes=%3 %4:%5>%6:%7")
					.arg(packet->buf_.size_).arg(item->packets).arg(item->bytes)
					.arg(QString(udpFlowKey->sip_)).arg(udpFlowKey->sport_).arg(QString(udpFlowKey->dip_)).arg(udpFlowKey->dport_);
			}
		}
	}

	emit debugged(packet);
}
