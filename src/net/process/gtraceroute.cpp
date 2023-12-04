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
	if (ipMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "ipMgr must be specified");
		return false;
	}
	ipOffset_ = ipMgr_->requestItems_.request(this, sizeof(Item));
	ipMgr_->managables_.insert(this);

	if (pcapDevice_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "pcapDevice is null");
		return false;
	}

	intf_ = pcapDevice_->intf();
	if (intf_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "intf is null");
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
			qDebug() << QString("testThreadSet count=%1").arg(count);
			if (count == 0) break;
		}
		QCoreApplication::processEvents();
		QThread::msleep(100);
		quint64 now = timer.elapsed();
		if (now - start > G::Timeout) {
			int count = testThreadMap_.count();
			qCritical() << QString("floodingThreadSet_.count() is %1").arg(count);
			break;
		}
	}
	return true;
}

void GTraceRoute::ipCreated(GIp ip, GIpMgr::IpValue* ipValue) {
	(void)ip;
	Item* item = PItem(ipValue->mem(ipOffset_));
	new (item) Item;
	qDebug() << "";
}

void GTraceRoute::ipDeleted(GIp ip, GIpMgr::IpValue* ipValue) {
	(void)ip;
	Item* item = PItem(ipValue->mem(ipOffset_));
	item->~Item();
	qDebug() << "";
}

void GTraceRoute::test(GPacket* packet) {
	(void)packet;
	Q_ASSERT(ipMgr_->currentIpVal_ != nullptr);
	Item* item = PItem(ipMgr_->currentIpVal_->mem(ipOffset_));
	if (item->active_) return;
	GIp ip = ipMgr_->currentIp_;
	QMetaObject::invokeMethod(this, [this, packet, ip]() {
		TestThread* thread = new TestThread(this, packet, ip);
		QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
		thread->start();
	}, Qt::QueuedConnection);
	item->active_ = true;
}

GTraceRoute::TestThread::TestThread(GTraceRoute* tr, GPacket* packet, GIp ip) : GThread(tr) {
	GDEBUG_CTOR
	tr_ = tr;
	ip_ = ip;
	packet_ = anyPacket_.get(packet->dlt());
	packetByteArray_.resize(packet->buf_.size_);
	GBuf buf(pbyte(packetByteArray_.data()), packetByteArray_.size());
	packet_->copyFrom(packet, buf);

	{
		QMutexLocker ml(&tr_->testThreadMap_);
		tr->testThreadMap_.insert(ip, this);
	}
}

GTraceRoute::TestThread::~TestThread() {
	GDEBUG_DTOR
	{
		QMutexLocker ml(&tr_->testThreadMap_);
		tr_->testThreadMap_.remove(ip_);
	}
}

void GTraceRoute::TestThread::run() {
	qDebug() << "run run run!!!";
}
