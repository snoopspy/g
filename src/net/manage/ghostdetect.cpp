#include "ghostdetect.h"
#include "net/pdu/gdhcphdr.h"

// ----------------------------------------------------------------------------
// GHostDetect
// ----------------------------------------------------------------------------
Q_INVOKABLE GHostDetect::GHostDetect(QObject* parent) : GStateObj(parent) {
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GIntf* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
}

GHostDetect::~GHostDetect() {
	close();
}

bool GHostDetect::doOpen() {
	if (!enabled_) return true;

	if (checkIp_ == false && checkArp_ == false && checkDhcp_ == false) {
		SET_ERR(GErr::Fail, "all checking ip, arp, dhcp are false");
		return false;
	}

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "intf is null");
		return false;
	}

	myIp_ = intf_->ip();
	myMac_ = intf_->mac();
	gwIp_ = intf_->gateway();

	return true;
}

bool GHostDetect::doClose() {
	if (!enabled_) return true;
	hosts_.clear();
	return true;
}

bool GHostDetect::processDhcp(GPacket* packet, GMac* mac, GIp* ip, QString* hostName) {
	GUdpHdr* udpHdr = packet->udpHdr_;
	if (udpHdr == nullptr) return false;

	if (!(udpHdr->sport() == 67 || udpHdr->dport() == 67)) return false;

	GBuf dhcp = packet->udpData_;
	if (dhcp.data_ == nullptr) return false;
	if (dhcp.size_ < sizeof(GDhcpHdr)) return false;
	GDhcpHdr* dhcpHdr = PDhcpHdr(dhcp.data_);

	bool ok = false;
	if (dhcpHdr->yourIp() != 0) { // DHCP Offer of DHCP ACK sent from server
		*mac = dhcpHdr->clientMac();
		*ip = dhcpHdr->yourIp();
		ok = true;
	}

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return false;
	gbyte* end = packet->buf_.data_ + packet->buf_.size_;
	GDhcpHdr::Option* option = dhcpHdr->first();
	while (true) {
		if (option->type_ == GDhcpHdr::RequestedIpAddress) {
			*ip = ntohl(*PIp(option->value()));
			*mac = ethHdr->smac();
			ok = true;
		} else if (option->type_ == GDhcpHdr::HostName) {
			QByteArray tmp(pchar(option->value()), option->len_);
			*hostName = QString(tmp);
		}
		option = option->next();
		if (option == nullptr) break;
		if (pbyte(option) >= end) break;
	}
	return ok;
}

bool GHostDetect::processArp(GEthHdr* ethHdr, GArpHdr* arpHdr, GMac* mac, GIp* ip) {
	if (ethHdr->smac() != arpHdr->smac()) {
		qDebug() << QString("ARP spoofing detected %1 %2 %3").arg(
			QString(ethHdr->smac()),
			QString(arpHdr->smac()),
			QString(arpHdr->sip()));
		return false;
	}


	*mac = arpHdr->smac();
	*ip = arpHdr->sip();
	return true;
}

bool GHostDetect::processIp(GEthHdr* ethHdr, GIpHdr* ipHdr, GMac* mac, GIp* ip) {
	GIp sip = ipHdr->sip();
	if (!intf_->isSameLanIp(sip)) return false;

	*mac = ethHdr->smac();
	*ip = sip;
	return true;
}

void GHostDetect::detect(GPacket* packet) {
	GMac mac;
	GIp ip;
	QString hostName;

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) return;

	mac = ethHdr->smac();
	if (mac == myMac_) return;

	bool detected = false;
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr != nullptr && ipHdr->sip() != myIp_) {
		if (checkDhcp_ && processDhcp(packet, &mac, &ip, &hostName))
			detected = true;
		else if (checkIp_ && processIp(ethHdr, ipHdr, &mac, &ip))
			detected = true;
	}

	GArpHdr* arpHdr = packet->arpHdr_;
	if (arpHdr != nullptr) {
		if (checkArp_ && processArp(ethHdr, arpHdr, &mac, &ip))
			detected = true;
	}

	if (!detected) return;
	if (ip == myIp_ || ip == gwIp_) return;

	if (hostName != "") {
		Host host(mac, ip, hostName);
		qDebug() << QString("%1 %2 %3").arg(QString(mac), QString(ip), hostName);
		emit hostDetected(&host);
	}

	HostMap::iterator newHost = hosts_.end();
	{
		QMutexLocker ml(&hosts_.m_);
		HostMap::iterator it = hosts_.find(mac);
		if (it == hosts_.end()) {
			Host host(mac, ip, hostName);
			host.lastAccess_ = et_.elapsed();
			newHost = hosts_.insert(mac, host);
		} else {
			it.value().lastAccess_ = et_.elapsed();
		}
	}
	if (newHost != hosts_.end()) {
		qDebug() << QString("%1 %2").arg(QString(mac), QString(ip));
		emit hostDetected(&newHost.value());
	}
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GHostDetect::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		return new GPropItemInterface(param);
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
