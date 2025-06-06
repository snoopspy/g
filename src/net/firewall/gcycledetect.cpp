#include "gcycledetect.h"

// ----------------------------------------------------------------------------
// GCycleItem
// ----------------------------------------------------------------------------
void GCycleItem::TimevalList::check(QString prefix) {
	int cnt = count();
	Q_ASSERT(cnt > 1);

	QString msg = prefix + " ";
	TimevalList::iterator it = begin();
	quint64 prev = it->tv_sec;
	quint64 sum = 0;
	for (it++; it != end(); it++) {
		quint64 now = it->tv_sec;
		quint64 i = now >= prev ? now - prev : prev - now;
		prev = now;
		sum += i;
		msg += QString::number(i) + " ";
	}

	avg_ = (double)sum / (cnt - 1);

	it = begin();
	prev = it->tv_sec;
	double diffSum = 0;
	for (it++; it != end(); it++) {
		quint64 now = it->tv_sec;
		quint64 i = now >= prev ? now - prev : prev - now;
		prev = now;
		double diff = std::abs(avg_ - i);
		diffSum += diff;
	}

	avgDiff_ = diffSum / (cnt - 1);
	// qDebug() << QString("%1 (%2)").arg(msg).arg(avgDiff_); // gilgil temp 2024.11.17
}

void GCycleItem::Quint64List::check(QString prefix) {
	int cnt = count();
	Q_ASSERT(cnt > 1);

	QString msg = prefix + " ";
	quint64 sum = 0;
	for (quint64 i: *this) {
		msg += QString::number(i) + " ";
		sum += i;
	}

	avg_ = (double)sum / cnt;

	double diffSum = 0;
	for (quint64 i : *this) {
		double diff = std::abs(avg_ - (double)i);
		diffSum += diff;
	}

	avgDiff_ = diffSum / cnt;
	//	qDebug() << QString("%1 (%2)").arg(msg).arg(avgDiff_); // gilgil temp 2024.11.17
}

// ----------------------------------------------------------------------------
// GCycleDetect
// ----------------------------------------------------------------------------
bool GCycleDetect::doOpen() {
	if (minCheckCount_ <= 1) {
		SET_ERR(GErr::Fail, "minCheckCount must be greater than one");
		return false;
	}
	if (tcpFlowMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "tcpFlowMgr is null");
		return false;
	}
	itemOffset_ = tcpFlowMgr_->requestItems_.request(this, sizeof(Item*));
	tcpFlowMgr_->managables_.insert(this);

	tcpMap_.clear();

	return true;
}

bool GCycleDetect::doClose() {
	return true;
}

void GCycleDetect::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	// qDebug() << QString("+++++ %1:%2 > %3:%4").arg(QString(tcpFlowKey.sip_)).arg(QString::number(tcpFlowKey.sport_)).arg(QString(tcpFlowKey.dip_)).arg(QString::number(tcpFlowKey.dport_)); // gilgil temp 2024.11.17
	Item* item = getItem(tcpFlowValue);
	if (tcpFlowValue->direction_ == GPacketMgr::ServerToClient) {
		GFlow::TcpFlowKey revTcpFlowKey = tcpFlowKey.reverse();
		GTcpFlowMgr::FlowMap::iterator it = tcpFlowMgr_->flowMap_.find(revTcpFlowKey);
		Q_ASSERT(it != tcpFlowMgr_->flowMap_.end());
		GTcpFlowMgr::TcpFlowValue* revTcpFlowValue = it.value();
		Item* revItem = getItem(revTcpFlowValue);
		Q_ASSERT(revItem != nullptr);
		new (item) Item(revItem->ttl_);
		return;
	}
	Q_ASSERT(tcpFlowMgr_->currentPacket_ != nullptr);
	GIpHdr* ipHdr = tcpFlowMgr_->currentPacket_->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	uint8_t ttl = ipHdr->ttl();
	new (item) Item(ttl);

	// qDebug() << QString("Key %1 > %2:%3 ttl=%4").arg(QString(tcpFlowKey.sip_)).arg(QString(tcpFlowKey.dip_)).arg(tcpFlowKey.dport_).arg(QString::number(ttl)); // gilgil temp 2024.11.17
	GCycleItemKey key(tcpFlowKey.sip_, tcpFlowKey.dip_, tcpFlowKey.dport_, ttl);
	GCycleMap::iterator it = tcpMap_.find(key);
	if (it == tcpMap_.end()) {
		GCycleItem citem;
		emit created(key, &citem);
		it = tcpMap_.insert(key, citem);
	}

	GCycleItem& citem = it.value();
	citem.firstTimes_.push_back(tcpFlowMgr_->currentPacket_->ts_);
	if (citem.firstTimes_.count() >= minCheckCount_)
		citem.firstTimes_.check("firstTimes");
	emit updated(key, &citem);
}

void GCycleDetect::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	// qDebug() << QString("----- %1:%2 > %3:%4").arg(QString(tcpFlowKey.sip_)).arg(QString::number(tcpFlowKey.sport_)).arg(QString(tcpFlowKey.dip_)).arg(QString::number(tcpFlowKey.dport_)); // gilgil temp 2024.11.17

	Item* item = getItem(tcpFlowValue);
	uint8_t ttl = item->ttl_;

	bool clientToServer = tcpFlowValue->direction_ == GPacketMgr::ClientToServer;
	if (!clientToServer)
		tcpFlowKey = tcpFlowKey.reverse();

	// qDebug() << QString("Key %1 > %2:%3 ttl=%4").arg(QString(tcpFlowKey.sip_)).arg(QString(tcpFlowKey.dip_)).arg(tcpFlowKey.dport_).arg(QString::number(ttl)); // gilgil temp 2024.11.17
	GCycleItemKey key(tcpFlowKey.sip_, tcpFlowKey.dip_, tcpFlowKey.dport_, ttl);
	GCycleMap::iterator it = tcpMap_.find(key);
	Q_ASSERT(it != tcpMap_.end());
	GCycleItem& citem = it.value();

	if (clientToServer) {
		citem.lastTimes_.push_back(tcpFlowValue->lastTime_);
		if (citem.lastTimes_.count() >= minCheckCount_)
			citem.lastTimes_.check("lastTimes");

		citem.txPackets_.push_back(tcpFlowValue->packets_);
		if (citem.txPackets_.count() >= minCheckCount_)
			citem.txPackets_.check("txPackets");

		citem.txBytes_.push_back(tcpFlowValue->bytes_);
		if (citem.txBytes_.count() >= minCheckCount_)
			citem.txBytes_.check("txBytes");

		emit updated(key, &citem);
	} else {
		citem.rxPackets_.push_back(tcpFlowValue->packets_);
		if (citem.rxPackets_.count() >= minCheckCount_)
			citem.rxPackets_.check("rxPackets");

		citem.rxBytes_.push_back(tcpFlowValue->bytes_);
		if (citem.rxBytes_.count() >= minCheckCount_)
			citem.rxBytes_.check("rxBytes_");

		emit updated(key, &citem);
	}
}

