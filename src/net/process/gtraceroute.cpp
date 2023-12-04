#include "gtraceroute.h"

// ----------------------------------------------------------------------------
// GTraceRoute
// ----------------------------------------------------------------------------
GTraceRoute::GTraceRoute(QObject* parent) : GStateObj(parent) {
	qDebug() << "";
}

GTraceRoute::~GTraceRoute() {
	qDebug() << "";
}

bool GTraceRoute::doOpen() {
	if (pcapDevice_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "pcapDevice is null");
		return false;
	}

	intf_ = pcapDevice_->intf();
	if (intf_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "intf is null");
		return false;
	}
	myIp_ = intf_->ip();

	if (ipMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "ipMgr is null");
		return false;
	}
	ipOffset_ = ipMgr_->requestItems_.request(this, sizeof(Item));
	ipMgr_->managables_.insert(this);

	if (rawIpSocketWrite_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "rawIpSocketWrite is null");
		return false;
	}

	return true;
}

bool GTraceRoute::doClose() {
	qDebug() << "";

	{
		QMutexLocker ml(&testThreadMap_);
		for (TestThread* thread: testThreadMap_) {
			thread->swe_.wakeAll();
		}
	}

	QElapsedTimer timer;
	quint64 start = timer.elapsed();
	while (true) {
		{
			QMutexLocker ml(&testThreadMap_);
			int count = testThreadMap_.count();
			qDebug() << QString("testThreadMap_.count()=%1").arg(count);
			if (count == 0) break;
		}
		QCoreApplication::processEvents();
		QThread::msleep(100);
		quint64 now = timer.elapsed();
		if (now - start > G::Timeout) {
			int count = testThreadMap_.count();
			qCritical() << QString("testThreadMap_.count()=%1").arg(count);
			break;
		}
	}
	return true;
}

void GTraceRoute::ipCreated(GIp ip, GIpMgr::IpValue* ipValue) {
	qDebug() << ""; // gilgil temp 2023.12.05
	(void)ip;
	Item* item = PItem(ipValue->mem(ipOffset_));
	new (item) Item;
}

void GTraceRoute::ipDeleted(GIp ip, GIpMgr::IpValue* ipValue) {
	qDebug() << ""; // gilgil temp 2023.12.05
	(void)ip;
	Item* item = PItem(ipValue->mem(ipOffset_));
	item->~Item();
}

void GTraceRoute::processPacketResponse(GPacket* packet, bool* ok) {
	(void)ok;

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GIp dip = ipHdr->dip();
	if (dip != myIp_) return;
	GIcmpHdr* icmpHdr = packet->icmpHdr_;
	if (icmpHdr != nullptr) {
		uint8_t type = icmpHdr->type();
		switch (type) {
			case GIcmpHdr::TtlExceeded: {
				GIcmpIpHdr* icmpIpHdr = PIcmpIpHdr(icmpHdr);
				GIpHdr* nextIpHdr = icmpIpHdr->ipHdr();
				// GIp nextSip = nextIpHdr->sip();
				// GIp nextDip = nextIpHdr->dip();
				// {
				// 	QMutexLocker ml(&testThreadMap_);
				// 	TestThreadMap::iterator it = testThreadMap_.find(nextDip);
				// 	if (it != testThreadMap_.end());

				// }
				qDebug() << QString("%1 %2 %3").arg(QString(ipHdr->sip())).arg(QString(nextIpHdr->dip())).arg(nextIpHdr->id());
			}
			default: break;
		}
	}
}

void GTraceRoute::processCreateThread(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GIp sip = ipHdr->sip();
	GIp dip = ipHdr->dip();
	if (dip == myIp_) return;
	if (dip != ipMgr_->currentIp_) return;
	Q_ASSERT(ipMgr_->currentIpVal_ != nullptr);
	Item* item = PItem(ipMgr_->currentIpVal_->mem(ipOffset_));
	if (!item->active_) {
		GFlow::IpFlowKey testFlowKey(sip, dip);
		QMetaObject::invokeMethod(this, [this, packet, testFlowKey]() {
			TestThread* thread = new TestThread(this, packet, testFlowKey);
			QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
			thread->start();
		}, Qt::BlockingQueuedConnection);
		item->active_ = true;
	}
}

void GTraceRoute::test(GPacket* packet) {
	bool ok = false;
	processPacketResponse(packet, &ok);
	if (ok) return;
	processCreateThread(packet);
}

GTraceRoute::TestThread::TestThread(GTraceRoute* tr, GPacket* packet, GFlow::IpFlowKey testFlowKey) : GThread(tr) {
	GDEBUG_CTOR
	testFlowKey_ = testFlowKey;

	sendPacket_ = anyPacket_.get(packet->dlt());
	sendPacketByteArray_.resize(packet->buf_.size_);
	memcpy(sendPacketByteArray_.data(), packet->buf_.data_, packet->buf_.size_);
	GBuf buf(pbyte(sendPacketByteArray_.data()), sendPacketByteArray_.size());
	sendPacket_->copyFrom(packet, buf);

	{
		QMutexLocker ml(&tr->testThreadMap_);
		tr->testThreadMap_.insert(testFlowKey, this);
	}
}

GTraceRoute::TestThread::~TestThread() {
	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);
	GDEBUG_DTOR
	{
		QMutexLocker ml(&tr->testThreadMap_);
		tr->testThreadMap_.remove(testFlowKey_);
	}
}

void GTraceRoute::TestThread::run() {
	qDebug() << "beg";

	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);

	if (swe_.wait(tr->startTimeout_)) return;
	GIpHdr* ipHdr = sendPacket_->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	for (int ttl = 1; ttl <= tr->maxHop_; ttl++) {
		for (int q = 0; q < tr->queryCount_; q++) {
			ipHdr->tos_ = 0x44;
			ipHdr->id_ = htons(ttl);
			ipHdr->ttl_ = ttl;
			ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
			tr->rawIpSocketWrite_->write(sendPacket_);
			swe_.wait(tr->sendTimeout_);
			if (!tr->active()) break;
		}
		if (!tr->active()) break;
		swe_.wait(tr->ttlChangeTimeout_);
	}
	if (tr->active()) swe_.wait(tr->stopTimeout_);

	qDebug() << "beg";
}
