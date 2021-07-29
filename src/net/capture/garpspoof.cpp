#include "garpspoof.h"

// ----------------------------------------------------------------------------
// GArpSpoof
// ----------------------------------------------------------------------------
GArpSpoof::GArpSpoof(QObject* parent) : GArpSpoofBaseDevice(parent) {
	mtu_ = GPacket::MtuSize;
}

GArpSpoof::~GArpSpoof() {
	close();
}

bool GArpSpoof::doOpen() {
	if (!enabled_) return true;

#ifdef Q_OS_WIN
	// taskkill /IM arprecover.exe /F
	QString program = "taskkill";
	QStringList arguments;
	arguments.append("/IM");
	arguments.append("arprecover.exe");
	arguments.append("/F");
	QProcess::startDetached(program, arguments);
#else // Q_OS_WIN
	QString program = "su";
	QStringList arguments;
	arguments.append("-c");
	arguments.append("pkill arprecover");
	QProcess::startDetached(program, arguments);
#endif // Q_OS_WIN

	// ----- by gilgil 2021.06.18 -----
	// intf_ is determined in GPcapDevice::doOpen or GRemovePcapDevice::doOpen.
	// To set filter, intf_ must be determined before open base class doOpen
#ifdef Q_OS_ANDROID
	intf_ = GRemoteNetInfo::instance(ip_, port_).interfaceList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::VALUE_IS_NULL, msg);
		return false;
	}
#else // Q_OS_ANDROID
	intf_ = GNetInfo::instance().interfaceList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::VALUE_IS_NULL, msg);
		return false;
	}
#endif // Q_OS_ANDROID
	// --------------------------------

	QString internalFilter;
	if (virtualMac_.isNull()) {
		myMac_ = intf()->mac();
		internalFilter = QString("!(ether src %1)").arg(QString(intf()->mac()));
	} else {
		myMac_ = virtualMac_;
		internalFilter = QString("!(ether src %1) && !(ether src %2)").arg(QString(intf()->mac()), QString(virtualMac_));
	}
	QString backupFilter = filter_;
	filter_ = internalFilter;
	if (!GArpSpoofBaseDevice::doOpen()) return false;
	filter_ = backupFilter;

	if (filter_ != "") {
		bpFilter_ = new GBpFilter(this);
		bpFilter_->filter_ = filter_;
		if (!bpFilter_->open()) {
			err = bpFilter_->err;
			delete bpFilter_;
			bpFilter_ = nullptr;
			return false;
		}
	}

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

	if (!atm_.allResolved()) {
		atm_.intfName_ = intfName_;
		if (!atm_.open()) {
			err = atm_.err;
			atm_.close();
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

	for (Flow& flow: flowList_) {
		flow.makePacket(&flow.infectPacket_, myMac_, true, GArpHdr::Reply);
		flow.makePacket(&flow.recoverRequestPacket_, myMac_, false, GArpHdr::Request);
		flow.makePacket(&flow.recoverReplyPacket_, myMac_, false, GArpHdr::Reply);
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

	sendArpRecoverAll();

	if (flowList_.count() > 0) {
		QString flowString;
		{
			QMutexLocker(&flowList_.m_);
			for (Flow& flow: flowList_)
				flowString += QString("%1 %2 %3 %4 ").arg(QString(flow.senderIp_), QString(flow.senderMac_), QString(flow.targetIp_), QString(flow.targetMac_));
		}
		flowString = flowString.left(flowString.length() - 1);

#ifdef Q_OS_WIN
		QString arprecoverFile = "arprecover.exe";
		if (QFile::exists(arprecoverFile)) {
			QString argument = QString("-i %1 %2 %3 %4 %5 %6 %7").
				arg(10).arg(
				intfName_, QString(intf_->gateway()), QString(intf_->mask()),
				QString(intf_->ip()), QString(intf_->mac()), flowString);
			QStringList arguments = argument.split(' ');
			qDebug() << arguments;
			QProcess::startDetached(arprecoverFile, arguments);
#else // Q_OS_WIN
		QString arprecoverFile = "arprecover";
		if (QFile::exists(arprecoverFile)) {
			QStringList arguments;
			arguments.append("-c");
			QString path = QDir::currentPath();
		#ifdef Q_OS_ANDROID
			QString preloadStr = " ";
			if (QFile::exists("/system/lib/libfakeioctl.so"))
				preloadStr = "export LD_PRELOAD=libfakeioctl.so;";
			QString run = QString("export LD_LIBRARY_PATH=%1; %2 %3/%4").arg(path + "/../lib", preloadStr, path, arprecoverFile);
		#else // Q_OS_ANDROID
			QString run = QString("%1/%2").arg(path, arprecoverFile);
		#endif  // Q_OS_ANDROID
			arguments.append(QString("%1 -i %2 %3 %4 %5 %6 %7 %8").
				arg(run).arg(10).arg(
				intfName_, QString(intf_->gateway()), QString(intf_->mask()),
				QString(intf_->ip()), QString(intf_->mac()), flowString));
			qDebug() << arguments;
			QProcess::startDetached("su", arguments);
#endif // Q_OS_WIN
		}
	}

	if (bpFilter_ != nullptr) {
		delete bpFilter_;
		bpFilter_ = nullptr;
	}

	return GArpSpoofBaseDevice::doClose();
}

GPacket::Result GArpSpoof::read(GPacket* packet) {
	while (true) {
		if (state_ != Opened)
			return GPacket::Fail;

		GPacket::Result res = GArpSpoofBaseDevice::read(packet);
		if (res == GPacket::Eof || res == GPacket::Fail) return res;
		if (res == GPacket::None) continue;

		GEthHdr* ethHdr = packet->ethHdr_;
		Q_ASSERT(ethHdr != nullptr);

		// attacker sending packet?
		GMac smac = ethHdr->smac();
		if (smac == myMac_) continue;
		processPacket(packet);
		if (smac.isBroadcast() || smac.isMulticast()) continue;

		uint16_t type = ethHdr->type();
		if (type == GEthHdr::Arp) {
			GArpHdr* arpHdr = packet->arpHdr_;
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
		} else if (type == GEthHdr::Ip4) {
			GIpHdr* ipHdr = packet->ipHdr_;
			Q_ASSERT(ipHdr != nullptr);
			GIp adjSrcIp = intf_->getAdjIp(ipHdr->sip());
			GIp adjDstIp = intf_->getAdjIp(ipHdr->dip());
			GFlow::IpFlowKey key(adjSrcIp, adjDstIp);
			FlowMap::iterator it = flowMap_.find(key);
			if (it == flowMap_.end()) continue;
			Flow& flow = it.value();
			ethHdr->dmac_ = flow.targetMac_;
			if (bpFilter_ != nullptr) {
				if (!bpFilter_->check(&packet->buf_)) {
					relay(packet);
					continue;
				}
			}
			return GPacket::Ok;
		}
		continue;
	}
	qWarning() << QString("unreachable routine state_=%1").arg(state_);
	return GPacket::Fail; // remove warning: non-void function does not return a value in all control paths
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

void GArpSpoof::Flow::makePacket(GEthArpHdr* packet, GMac myMac, bool infect, uint16_t operation) {
	packet->ethHdr_.dmac_ = senderMac_;
	packet->ethHdr_.smac_ = myMac;
	packet->ethHdr_.type_ = htons(GEthHdr::Arp);

	packet->arpHdr_.hrd_ = htons(GArpHdr::ETHER);
	packet->arpHdr_.pro_ = htons(GEthHdr::Ip4);
	packet->arpHdr_.hln_ = sizeof(GMac);
	packet->arpHdr_.pln_ = sizeof(GIp);
	packet->arpHdr_.op_ = htons(operation);
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

bool GArpSpoof::sendArpRecoverAll() {
	QMutexLocker(&flowList_.m_);
	for (Flow& flow: flowList_) {
		if (!sendArpRecover(&flow))
			return false;
		QThread::msleep(sendInterval_);
	}
	return true;
}

bool GArpSpoof::sendArpRecover(Flow* flow) {
	bool res = true;
	GPacket::Result written = write(GBuf(pbyte(&flow->recoverRequestPacket_), sizeof(flow->recoverRequestPacket_)));
	if (written != GPacket::Ok)  res = false;
	written = write(GBuf(pbyte(&flow->recoverReplyPacket_), sizeof(flow->recoverReplyPacket_)));
	if (written != GPacket::Ok)  res = false;
	return res;
}

void GArpSpoof::processPacket(GPacket* packet) {
	(void)packet;
}
