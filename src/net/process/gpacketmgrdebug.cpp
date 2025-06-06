#include "gpacketmgrdebug.h"

// ----------------------------------------------------------------------------
// GPacketMgrDebug
// ----------------------------------------------------------------------------
bool GPacketMgrDebug::doOpen() {
	if (hostMgr_ != nullptr) {
		hostOffset_ = hostMgr_->requestItems_.request(this, sizeof(Item));
		hostMgr_->managables_.insert(this);
	}

	if (ipFlowMgr_ != nullptr) {
		ipFlowOffset_ = ipFlowMgr_->requestItems_.request(this, sizeof(Item));
		ipFlowMgr_->managables_.insert(this);
	}

	if (tcpFlowMgr_ != nullptr) {
		tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request(this, sizeof(Item));
		tcpFlowMgr_->managables_.insert(this);
	}

	if (udpFlowMgr_ != nullptr) {
		udpFlowOffset_ = udpFlowMgr_->requestItems_.request(this, sizeof(Item));
		udpFlowMgr_->managables_.insert(this);
	}
	return true;
}

bool GPacketMgrDebug::doClose() {
	return true;
}

void GPacketMgrDebug::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	if (!enabled_) return;
	qDebug() << QString("hostCreated %1 %2 %3").arg(QString(mac)).arg(QString(hostValue->ip_)).arg(hostValue->host_);
	Item* item = getItem(hostValue);
	new (item) Item;
}

void GPacketMgrDebug::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	if (!enabled_) return;
	qDebug() << QString("hostDeleted %1 %2 %3").arg(QString(mac)).arg(QString(hostValue->ip_)).arg(hostValue->host_);
	Item* item = getItem(hostValue);
	item->~Item();
}

void GPacketMgrDebug::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	if (!enabled_) return;
	qDebug() << QString("hostDeleted %1 %2").arg(QString(mac)).arg(QString(hostValue->ip_)).arg(hostValue->host_);
}

void GPacketMgrDebug::ipFlowCreated(GFlow::IpFlowKey ipFlowKey, GIpFlowMgr::IpFlowValue* ipFlowValue) {
	if (!enabled_) return;
	qDebug() << QString("ipFlowCreated %1>%2").arg(QString(ipFlowKey.sip_)).arg(QString(ipFlowKey.dip_));
	Item* item = getItem(ipFlowValue);
	new (item) Item;
}

void GPacketMgrDebug::ipFlowDeleted(GFlow::IpFlowKey ipFlowKey, GIpFlowMgr::IpFlowValue* ipFlowValue) {
	if (!enabled_) return;
	qDebug() << QString("ipFlowDeleted %1>%2").arg(QString(ipFlowKey.sip_)).arg(QString(ipFlowKey.dip_));
	Item* item = getItem(ipFlowValue);
	item->~Item();
}

void GPacketMgrDebug::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	if (!enabled_) return;
	qDebug() << QString("tcpFlowCreated %1").arg(QString(tcpFlowKey));
	Item* item = getItem(tcpFlowValue);
	new (item) Item;
}

void GPacketMgrDebug::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	if (!enabled_) return;
	qDebug() << QString("tcpFlowDeleted %1:%2>%3:%4").arg(QString(tcpFlowKey.sip_)).arg(tcpFlowKey.sport_).arg(QString(tcpFlowKey.dip_)).arg(tcpFlowKey.dport_);
	Item* item = getItem(tcpFlowValue);
	item->~Item();
}

void GPacketMgrDebug::udpFlowCreated(GFlow::UdpFlowKey udpFlowKey, GUdpFlowMgr::UdpFlowValue* udpFlowValue) {
	if (!enabled_) return;
	qDebug() << QString("udpFlowCreated %1:%2>%3:%4").arg(QString(udpFlowKey.sip_)).arg(udpFlowKey.sport_).arg(QString(udpFlowKey.dip_)).arg(udpFlowKey.dport_);
	Item* item = getItem(udpFlowValue);
	new (item) Item;
}

void GPacketMgrDebug::udpFlowDeleted(GFlow::UdpFlowKey udpFlowKey, GUdpFlowMgr::UdpFlowValue* udpFlowValue) {
	if (!enabled_) return;
	qDebug() << QString("udpFlowDeleted %1:%2>%3:%4").arg(QString(udpFlowKey.sip_)).arg(udpFlowKey.sport_).arg(QString(udpFlowKey.dip_)).arg(udpFlowKey.dport_);
	Item* item = getItem(udpFlowValue);
	item->~Item();
}

void GPacketMgrDebug::debugHost(GPacket* packet) {
	if (!enabled_) return;

	Q_ASSERT(hostMgr_ != nullptr);
	Q_ASSERT(hostMgr_->currentHostVal_ != nullptr);

	Item* item = getItem(hostMgr_->currentHostVal_);
	item->packets++;
	item->bytes += packet->buf_.size_;
	GMac mac = hostMgr_->currentMac_;
	qDebug() << QString("host size=%1 packets=%2 bytes=%3 %4")
		.arg(packet->buf_.size_).arg(item->packets).arg(item->bytes)
		.arg(QString(mac));

	emit debugged(packet);
}

void GPacketMgrDebug::debugIp(GPacket* packet) {
	if (!enabled_) return;

	Q_ASSERT(ipFlowMgr_ != nullptr);
	Q_ASSERT(ipFlowMgr_->currentIpFlowVal_ != nullptr);

	Item* item = getItem(ipFlowMgr_->currentIpFlowVal_);
	item->packets++;
	item->bytes += packet->buf_.size_;
	GFlow::IpFlowKey* ipFlowKey = &ipFlowMgr_->currentIpFlowKey_;
	qDebug() << QString("ip size=%1 packets=%2 bytes=%3 %4>%5")
		.arg(packet->buf_.size_).arg(item->packets).arg(item->bytes)
		.arg(QString(ipFlowKey->sip_)).arg(QString(ipFlowKey->dip_));

	emit debugged(packet);
}

void GPacketMgrDebug::debugTcp(GPacket* packet) {
	if (!enabled_) return;

	Q_ASSERT(tcpFlowMgr_ != nullptr);
	Q_ASSERT(tcpFlowMgr_->currentTcpFlowVal_ != nullptr);

	Item* item = getItem(tcpFlowMgr_->currentTcpFlowVal_);
	item->packets++;
	item->bytes += packet->buf_.size_;
	GFlow::TcpFlowKey* tcpFlowKey = &tcpFlowMgr_->currentTcpFlowKey_;
	qDebug() << QString("tcp size=%1 packets=%2 bytes=%3 %4:%5>%6:%7")
		.arg(packet->buf_.size_).arg(item->packets).arg(item->bytes)
		.arg(QString(tcpFlowKey->sip_)).arg(tcpFlowKey->sport_).arg(QString(tcpFlowKey->dip_)).arg(tcpFlowKey->dport_);

	emit debugged(packet);
}

void GPacketMgrDebug::debugUdp(GPacket* packet) {
	if (!enabled_) return;

	Q_ASSERT(udpFlowMgr_ != nullptr);
	Q_ASSERT(udpFlowMgr_->currentUdpFlowVal_ != nullptr);

	Item* item = getItem(udpFlowMgr_->currentUdpFlowVal_);
	item->packets++;
	item->bytes += packet->buf_.size_;
	GFlow::UdpFlowKey* udpFlowKey = &udpFlowMgr_->currentUdpFlowkey_;
	qDebug() << QString("udp size=%1 packets=%2 bytes=%3 %4:%5>%6:%7")
		.arg(packet->buf_.size_).arg(item->packets).arg(item->bytes)
		.arg(QString(udpFlowKey->sip_)).arg(udpFlowKey->sport_).arg(QString(udpFlowKey->dip_)).arg(udpFlowKey->dport_);

	emit debugged(packet);
}
