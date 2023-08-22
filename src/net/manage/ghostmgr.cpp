#include "ghostmgr.h"
#include "net/pdu/gdhcphdr.h"

// ----------------------------------------------------------------------------
// GHostMgr
// ----------------------------------------------------------------------------
bool GHostMgr::doOpen() {
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
	if (myIp_ == 0) {
		SET_ERR(GErr::ValueIsZero, "ip is zero");
		return false;
	}
	myMac_ = intf_->mac();
	if (myMac_.isNull()) {
		SET_ERR(GErr::ValueIsZero, "mac is zero");
		return false;
	}
	gwIp_ = intf_->gateway();

	{
		QMutexLocker ml(&hostMap_.m_);
		hostMap_.clear();
	}
	return GPacketMgr::doOpen();
}

bool GHostMgr::doClose() {
	{
		QMutexLocker ml(&hostMap_.m_);
		for (Managable* manager: managables_) {
			for (HostMap::iterator it = hostMap_.begin(); it != hostMap_.end(); it++) {
				GMac mac = it.key();
				HostValue* hostValue = it.value();
				manager->hostDeleted(mac, hostValue);
			}
		}
		hostMap_.clear();
	}
	return GPacketMgr::doClose();
}

void GHostMgr::deleteOldHosts(time_t now) {
	QMutexLocker ml(&hostMap_.m_);
	HostMap::iterator it = hostMap_.begin();
	while (it != hostMap_.end()) {
		HostValue* hostValue = it.value();
		long elapsed = now - hostValue->lastTs_.tv_sec;
		if (elapsed >= timeoutSec_) {
			GMac mac = it.key();
			qDebug() << QString("%1 %2").arg(QString(mac)).arg(QString(it.value()->ip_));
			for (Managable* manager: managables_)
				manager->hostDeleted(mac, hostValue);
			it = hostMap_.erase(it);
			continue;
		}
		it++;
	}
}

bool GHostMgr::processDhcp(GPacket* packet, GMac* mac, GIp* ip, QString* host, QString* alias) {
	GUdpHdr* udpHdr = packet->udpHdr_;
	if (udpHdr == nullptr) return false;

	if (!(udpHdr->sport() == 67 || udpHdr->dport() == 67)) return false;

	GBuf dhcp = packet->udpData_;
	if (dhcp.data_ == nullptr) return false;
	if (dhcp.size_ < sizeof(GDhcpHdr)) return false;
	GDhcpHdr* dhcpHdr = PDhcpHdr(dhcp.data_);

	bool ok = false;
	if (!dhcpHdr->clientMac().isNull()) { // Discover, Offer, Request, ACK
		*mac = dhcpHdr->clientMac();
		ok = true;
	}
	if (dhcpHdr->yourIp() != 0) { // Offer, ACK sent from server
		*ip = dhcpHdr->yourIp();
	}

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return false;
	gbyte* end = packet->buf_.data_ + packet->buf_.size_;
	GDhcpHdr::Option* option = dhcpHdr->firstOption();
	bool exit = false;
	while (!exit) {
		switch (option->type_) {
			case GDhcpHdr::RequestedIpAddress: // Request
				*ip = ntohl(*PIp(option->value()));
				break;
			case GDhcpHdr::HostName: // Discover, Request sent from client
				*host = std::string(reinterpret_cast<const char*>(option->value()), option->len_).data();
				break;
			case GDhcpHdr::VendorClassIdentitier:
				*alias = std::string(reinterpret_cast<const char*>(option->value()), option->len_).data();
				break;
			case GDhcpHdr::End:
				exit = true;
				break;
			default:
				break;
		}
		if (exit) break;
		option = option->next();
		if (option == nullptr) break;
		if (pbyte(option) >= end) break;
	}
	return ok;
}

bool GHostMgr::processArp(GEthHdr* ethHdr, GArpHdr* arpHdr, GMac* mac, GIp* ip) {
	*mac = arpHdr->smac();
	*ip = arpHdr->sip();
	return true;
}

bool GHostMgr::processIp(GEthHdr* ethHdr, GIpHdr* ipHdr, GMac* mac, GIp* ip) {
	GIp sip = ipHdr->sip();
	if (!intf_->isSameLanIp(sip)) return false;

	*mac = ethHdr->smac();
	*ip = sip;
	return true;
}

void GHostMgr::manage(GPacket* packet) {
	time_t now = packet->ts_.tv_sec;
	if (checkIntervalSec_ != 0 && now - lastCheckClock_ >= checkIntervalSec_) {
		deleteOldHosts(now);
		lastCheckClock_ = now;
	}

	GMac mac(GMac::nullMac());
	GIp ip(0);
	QString host;
	QString vendor;

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return;

	mac = ethHdr->smac();
	if (mac == myMac_) return;

	bool detected = false;
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr != nullptr && ipHdr->sip() != myIp_) {
		if (processDhcp(packet, &mac, &ip, &host, &vendor) || processIp(ethHdr, ipHdr, &mac, &ip))
		detected = true;
	}

	GArpHdr* arpHdr = packet->arpHdr_;
	if (arpHdr != nullptr && arpHdr->sip() != myIp_) {
		if (processArp(ethHdr, arpHdr, &mac, &ip))
			detected = true;
	}

	if (!detected) return;
	if (ip == myIp_ || ip == gwIp_) return;

	currentMac_ = mac;
	{
		QMutexLocker ml(&hostMap_.m_);
		HostMap::iterator it = hostMap_.find(currentMac_);
		if (it == hostMap_.end()) {
			qDebug() << QString("detected %1 %2 %3 %4").arg(QString(mac)).arg(QString(ip)).arg(host).arg(vendor); // gilgil temp 2022.03.07
			currentHostVal_ = HostValue::allocate(requestItems_.totalMemSize_);
			currentHostVal_->firstTs_ = currentHostVal_->lastTs_ = packet->ts_;
			currentHostVal_->ip_ = ip;
			currentHostVal_->host_ = host;
			currentHostVal_->vendor_ = vendor;

			it = hostMap_.insert(currentMac_, currentHostVal_);
			for (Managable* manager: managables_)
				manager->hostCreated(currentMac_, currentHostVal_);
		} else {
			bool changed = false;
			HostValue* hv = it.value();
			if (ip != 0 && ip != hv->ip_) {
				hv->ip_ = ip;
				changed = true;
			}
			if (host != "" && host != hv->host_) {
				hv->host_ = host;
				changed = true;
			}
			if (vendor != "" && vendor != hv->vendor_) {
				hv->vendor_ = vendor;
				changed = true;
			}
			currentHostVal_ = it.value();
			if (changed) {
				qDebug() << QString("changed %1 %2 %3 %4").arg(QString(it.key())).arg(QString(hv->ip_)).arg(hv->host_).arg(hv->vendor_); // gilgil temp 2022.03.07
				for (Managable* manager: managables_)
					manager->hostChanged(currentMac_, currentHostVal_);
			}
		}
	}
	Q_ASSERT(currentHostVal_ != nullptr);
	currentHostVal_->lastTs_ = packet->ts_;

	emit managed(packet);
}

bool GHostMgr::propLoad(QJsonObject jo, QMetaProperty mpro) {
	// qDebug() << mpro.name(); // gilgil temp 2021.11.11
	if (QString(mpro.name()) == "pcapDevice") {
		QObject* p = parent();
		if (p != nullptr && QString(p->metaObject()->className()) == "GAutoArpSpoof")
			return true;
	}
	return GStateObj::propLoad(jo, mpro);
}
