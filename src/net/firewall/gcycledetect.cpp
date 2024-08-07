#include "gcycledetect.h"

// ----------------------------------------------------------------------------
// GCycleItem
// ----------------------------------------------------------------------------
void GCycleItem::TimevalList::check(QString prefix) {
	QString msg = prefix + " ";
	quint64 sum = 0;
	for (struct timeval& tv: *this) {
		msg += QString::number(tv.tv_sec) + " ";
		sum += tv.tv_sec;
	}
	double avg = sum / count();
	double diffSum = 0;
	for (struct timeval& tv: *this) {
		double diff = std::abs(avg - (double)tv.tv_sec);
		diffSum += diff;
	}
	diffAvg_ = diffSum / count();
	qDebug() << QString("%1:%2").arg(msg).arg(diffAvg_);
}

void GCycleItem::Quint64List::check(QString prefix) {
	QString msg = prefix + " ";
	quint64 sum = 0;
	for (quint64 i : *this) {
		msg += QString::number(i) + " ";
		sum += i;
	}
	double avg = sum / count();
	double diffSum = 0;
	for (quint64 i : *this) {
		double diff = std::abs(avg - (double)i);
		diffSum += diff;
	}
	diffAvg_ = diffSum / count();
	qDebug() << QString("%1:%2").arg(msg).arg(diffAvg_);
}

// ----------------------------------------------------------------------------
// GCycleDetect
// ----------------------------------------------------------------------------
bool GCycleDetect::doOpen() {
	if (tcpFlowMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "tcpFlowMgr is null");
		return false;
	}
	tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request(this, sizeof(Item*));
	tcpFlowMgr_->managables_.insert(this);

	return true;
}

bool GCycleDetect::doClose() {
	tcpMap_.clear();
	return true;
}

void GCycleDetect::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	qDebug() << QString("%1:%2 > %3:%4").arg(QString(tcpFlowKey.sip_)).arg(QString::number(tcpFlowKey.sport_)).arg(QString(tcpFlowKey.dip_)).arg(QString::number(tcpFlowKey.dport_));
	Item* item = getItem(tcpFlowValue);
	if (tcpFlowValue->direction_ == GPacketMgr::ServerToClient) {
		new (item) Item(0);
		return;
	}
	Q_ASSERT(tcpFlowMgr_->currentPacket_ != nullptr);
	GIpHdr* ipHdr = tcpFlowMgr_->currentPacket_->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	uint8_t ttl = ipHdr->ttl();
	new (item) Item(ttl);

	qDebug() << QString("Key %1 > %2 ttl=%3").arg(QString(tcpFlowKey.sip_)).arg(QString(tcpFlowKey.dip_)).arg(QString::number(ttl));
	GCycleItemKey key(tcpFlowKey.sip_, tcpFlowKey.dip_, tcpFlowKey.dport_, ttl);
	GCycleMap::iterator it = tcpMap_.find(key);
	if (it == tcpMap_.end()) {
		GCycleItem  citem;
		it = tcpMap_.insert(key, citem);
	}
	GCycleItem& citem = it.value();
	citem.firstTimes_.push_back(tcpFlowMgr_->currentPacket_->ts_);
	citem.firstTimes_.check("firstTimes");
}

void GCycleDetect::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	qDebug() << QString("%1:%2 > %3:%4").arg(QString(tcpFlowKey.sip_)).arg(QString::number(tcpFlowKey.sport_)).arg(QString(tcpFlowKey.dip_)).arg(QString::number(tcpFlowKey.dport_));
	Item* item = getItem(tcpFlowValue);
	bool clientToServer = tcpFlowValue->direction_ == GPacketMgr::ClientToServer;
	if (!clientToServer) {
		tcpFlowKey = tcpFlowKey.reverse();
		tcpFlowMgr_->flowMap_.find(tcpFlowKey);
		if (tcpFlowValue == nullptr) return;
		item = getItem(tcpFlowValue);
	}

	uint8_t ttl = item->ttl_;
	qDebug() << QString("Key %1 > %2 ttl=%3").arg(QString(tcpFlowKey.sip_)).arg(QString(tcpFlowKey.dip_)).arg(QString::number(ttl));
	GCycleItemKey key(tcpFlowKey.sip_, tcpFlowKey.dip_, tcpFlowKey.dport_, ttl);
	GCycleMap::iterator it = tcpMap_.find(key);
	if (it == tcpMap_.end()) {
		GCycleItem citem;
		it = tcpMap_.insert(key, citem);
	}
	GCycleItem& citem = it.value();
	if (clientToServer) {
		citem.lastTimes_.push_back(tcpFlowValue->lastTime_);
		citem.lastTimes_.check("lastTimes");

		citem.txPackets_.push_back(tcpFlowValue->packets_);
		citem.txPackets_.check("txPackets");

		citem.txBytes_.push_back(tcpFlowValue->bytes_);
		citem.txBytes_.check("txBytes");
	} else {
		citem.rxPackets_.push_back(tcpFlowValue->packets_);
		citem.rxPackets_.check("rxPackets");

		citem.rxBytes_.push_back(tcpFlowValue->bytes_);
		citem.rxBytes_.check("rxBytes_");
	}
}

void GCycleDetect::detect(GPacket* packet) {
	(void)packet;
}
