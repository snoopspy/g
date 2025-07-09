#include <QDebug>
#include "gintf.h"
#include "grtm.h"
#ifdef Q_OS_WIN
#include "net/_win/gipadapterinfo.h"
#endif

// ----------------------------------------------------------------------------
// GIntf
// ----------------------------------------------------------------------------
bool GIntf::operator==(const GIntf& r) const {
	if (index_ != r.index_) return false;
	if (name_ != r.name_) return false;
	if (desc_ != r.desc_) return false;
	if (mac_ != r.mac_) return false;
	if (ip_ != r.ip_) return false;
	if (mask_ != r.mask_) return false;
	if (gateway_ != r.gateway_) return false;
	return true;
}

// ----------------------------------------------------------------------------
// GIntfList
// ----------------------------------------------------------------------------
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include <net/if.h> // for ifreq
#include <sys/ioctl.h> // for SIOCGIFHWADDR
static GMac getMac(char* intfName) {
	int s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		qDebug() << "socket return -1" << strerror(errno);
		return GMac::nullMac();
	}

	struct ifreq buffer;
	memset(&buffer, 0x00, sizeof(buffer));
	strncpy(buffer.ifr_name, intfName, IFNAMSIZ - 1);

	int i = ioctl(s, SIOCGIFHWADDR, &buffer);
	close(s);
	if (i == -1) {
		// qDebug() << "ioctl return -1" << intfName << strerror(errno);
		return GMac::nullMac();
	}

	const u_char* p = const_cast<const u_char*>(puchar(buffer.ifr_ifru.ifru_hwaddr.sa_data));
	GMac res(p);
	return res;
}
#endif

#if (defined(Q_OS_LINUX) || defined(Q_OS_WIN)) && !defined(Q_OS_ANDROID)
void GIntfList::init() {
	clear();

	//
	// Initialize allDevs using pcap API.
	//
	pcap_if_t* allDevs;
	char errBuf[PCAP_ERRBUF_SIZE];
	int i = pcap_findalldevs(&allDevs, errBuf);
	if (i != 0) { // if error occured
		qWarning() << QString("error in pcap_findalldevs (%1)").arg(errBuf);
		return;
	}

	//
	// Add all interfaces
	//
	pcap_if_t* dev = allDevs;
	i = 1;
	while (dev != nullptr) {
		GIntf intf;

		intf.index_ = i;
		intf.name_ = dev->name;
		intf.desc_ = dev->description != nullptr ? dev->description : intf.name_;
		if (intf.desc_ == "") intf.desc_ = intf.name_;

#ifdef Q_OS_LINUX
		intf.mac_ = getMac(dev->name);
		for(pcap_addr_t* pa = dev->addresses; pa != nullptr; pa = pa->next) {
			struct sockaddr* addr = pa->addr;
			struct sockaddr_in* addr_in = reinterpret_cast<struct sockaddr_in*>(addr);
			if(addr != nullptr && addr->sa_family == AF_INET)
				intf.ip_ = ntohl(addr_in->sin_addr.s_addr);

			addr = pa->netmask;
			addr_in = reinterpret_cast<struct sockaddr_in*>(addr);
			if(addr != nullptr && addr->sa_family == AF_INET) {
				intf.mask_ = ntohl(addr_in->sin_addr.s_addr);
			}
		}
#endif
#ifdef Q_OS_WIN
		PIP_ADAPTER_INFO adapter = GIpAdapterInfo::instance().findByAdapterName(dev->name);
		if (adapter != nullptr) {
			intf.desc_ = adapter->Description;
			if (adapter->AddressLength == GMac::Size)
				intf.mac_ = adapter->Address;
			intf.ip_ = QString(adapter->IpAddressList.IpAddress.String);
			intf.mask_ = QString(adapter->IpAddressList.IpMask.String);
			intf.gateway_ = QString(adapter->GatewayList.IpAddress.String);
		}
#endif
		intf.ip_and_mask_ = intf.ip_ & intf.mask_;
		// gateway_ is initialized in GNetInfo
		push_back(intf);
		dev = dev->next;
		i++;
	}

	pcap_freealldevs(allDevs);
}
#endif

#ifdef Q_OS_ANDROID
#include "net/demon/gdemonclient.h"

void GIntfList::init() {
	clear();

	GDemonClient& client = GDemonClient::instance("127.0.0.1", GDemonClient::DefaultPort);
	GDemon::GetInterfaceListRes res = client.getInterfaceList();
	for (GDemon::Interface& dIntf: res.interfaceList_) {
		GIntf intf;
		intf.index_ = dIntf.index_;
		intf.name_ = dIntf.name_.data();
		intf.desc_ = dIntf.desc_.data();
		intf.mac_ = dIntf.mac_;
		intf.ip_ = dIntf.ip_;
		intf.mask_ = dIntf.mask_;
		intf.ip_and_mask_ = intf.ip_ & intf.mask_;
		// gateway_ is initialized in GNetInfo
		push_back(intf);
	}
}
#endif

#ifdef Q_OS_LINUX
GIntf* GIntfList::findByName(QString name) {
	for (GIntf& intf: *this) {
		if (intf.name() == name)
			return &intf;
	}
	return nullptr;
}
#endif
#ifdef Q_OS_WIN
GIntf* GIntfList::findByName(QString name) {
	for (GIntf& intf: *this) {
		if (intf.name().indexOf(name) != -1)
			return &intf;
	}
	return nullptr;
}
#endif

GIntf* GIntfList::findByIp(GIp ip) {
	for (GIntf& intf: *this) {
		if (intf.ip() == ip)
			return &intf;
	}
	return nullptr;
}
