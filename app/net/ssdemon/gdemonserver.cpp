#include "gdemonserver.h"
#include "gtrace.h"

// ----------------------------------------------------------------------------
// GDemonServer
// ----------------------------------------------------------------------------
GDemonServer::GDemonServer() {
}

GDemonServer::~GDemonServer() {
	stop();
	wait();
}

bool GDemonServer::start(uint16_t port) {
	accept_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (accept_ == -1) {
		GTRACE("%s", strerror(errno));
		return false;
	}

	int optval = 1;
	int res = ::setsockopt(accept_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		GTRACE("%s", strerror(errno));
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	res = ::bind(accept_, (struct sockaddr *)&addr, sizeof(addr));
	if (res == -1) {
		GTRACE("%s", strerror(errno));
		return false;
	}

	res = listen(accept_, 5);
	if (res == -1) {
		GTRACE("%s", strerror(errno));
		return false;
	}

	return true;
}

void GDemonServer::exec() {
	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int new_sd = accept(accept_, (struct sockaddr *)&cli_addr, &len);
		if (new_sd == -1) {
			GTRACE("%s", strerror(errno));
			break;
		}
		std::thread* t = new std::thread(GDemonSession::_run, this, new_sd);
		t->detach();
	}
}

void GDemonServer::stop() {
	if (accept_ != 0) {
		close(accept_);
		accept_ = 0;
	}

	sessions_.lock();
	for (GDemonSession* session: sessions_) {
		::close(session->sd_);
	}
	sessions_.unlock();
}

void GDemonServer::wait() {
	while(true) {
		sessions_.lock();
		bool exit = sessions_.size() == 0;
		sessions_.unlock();
		if (exit) break;
		sleep(1);
	}
}

// ----------------------------------------------------------------------------
// GDemonSession
// ----------------------------------------------------------------------------
GDemonSession::GDemonSession(GDemonServer* server) : server_(server) {
}

GDemonSession::~GDemonSession() {
	if (sd_ != 0) {
		::close(sd_);
		sd_ = 0;
	}

	if (network_ != nullptr) {
		delete network_;
		network_ = nullptr;
	}

	if (pcap_ != nullptr) {
		delete pcap_;
		pcap_ = nullptr;
	}
}

void GDemonSession::_run(GDemonServer* server, int new_sd) {
	GDemonSession* session = new GDemonSession(server);
	session->sd_ = new_sd;

	server->sessions_.lock();
	server->sessions_.push_back(session);
	server->sessions_.unlock();

	session->run();

	server->sessions_.lock();
	server->sessions_.remove(session);
	server->sessions_.unlock();

	delete session;
}

void GDemonSession::run() {
	GTRACE("beg");

	bool active = true;
	while (active) {
		char buffer[MaxBufferSize];
		Header* header = GDemon::PHeader(buffer);
		if (!recvAll(sd_, header, sizeof(Header)))
			break;
		if (header->len_ < 0 || header->len_ > MaxBufferSize) {
			GTRACE("invalid len_ %d", header->len_);
			break;
		}

		if (!recvAll(sd_, buffer + sizeof(Header), header->len_))
			break;

		pchar buf = buffer;
		int size = header->len_ + sizeof(Header);
		switch (header->cmd_) {
			case CmdGetInterfaceList:
				if (network_ == nullptr) network_ = new GDemonNetwork(this);
				active = network_->processGetInterfaceList(buf, size);
				break;
			case CmdGetRtm:
				if (network_ == nullptr) network_ = new GDemonNetwork(this);
				active = network_->processGetRtm(buf, size);
				break;
			case CmdPcapOpen:
				if (pcap_ == nullptr) pcap_ = new GDemonPcap(this);
				active = pcap_->processPcapOpen(buf, size);
				break;
			case CmdPcapClose:
				if (pcap_ == nullptr) pcap_ = new GDemonPcap(this);
				active = pcap_->processPcapClose(buf, size);
				break;
			case CmdPcapWrite:
				if (pcap_ == nullptr) pcap_ = new GDemonPcap(this);
				active = pcap_->processPcapWrite(buf, size);
				break;
			default:
				GTRACE("invalid cmd %d", header->cmd_);
				active = false;
				break;
		}
	}

	GTRACE("end");
}

// ----------------------------------------------------------------------------
// GDemonNetwork
// ----------------------------------------------------------------------------
GDemonNetwork::GDemonNetwork(GDemonSession* session) : session_(session) {
}

GDemonNetwork::~GDemonNetwork() {
}

#include <net/if.h> // for ifreq
#include <sys/ioctl.h> // for SIOCGIFHWADDR
bool GDemonNetwork::getMac(char* devName, uint8_t* mac) {
	int s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		GTRACE("socket return -1 %s", strerror(errno));
		return false;
	}

	struct ifreq buffer;
	memset(&buffer, 0x00, sizeof(buffer));
	strncpy(buffer.ifr_name, devName, IFNAMSIZ - 1);

	ioctl(s, SIOCGIFHWADDR, &buffer);
	close(s);

	memcpy(mac, buffer.ifr_ifru.ifru_hwaddr.sa_data, GDemon::Interface::MacSize);
	return true;
}

bool GDemonNetwork::processGetInterfaceList(pchar buf, int32_t size) {
	(void)buf; (void)size;

	GTRACE("");

	pcap_if_t* allDevs;
	char errBuf[PCAP_ERRBUF_SIZE];
	// GTRACE("bef call pcap_findalldevs"); // gilgil temp 2021.03.11
	int i = pcap_findalldevs(&allDevs, errBuf);
	// GTRACE("aft call pcap_findalldevs i=%d", i); // gilgil temp 2021.03.11
	if (i != 0) { // if error occured
		GTRACE("error in pcap_findalldevs_ex (%s)", errBuf);
		return false;
	}

	//
	// Add all interfaces
	//
	pcap_if_t* dev = allDevs;
	GetInterfaceListRep rep;
	i = 1;
	while (dev != nullptr) {
		Interface interface;

		interface.index_ = i;
		interface.name_ = dev->name;
		interface.desc_ = dev->description != nullptr ? dev->description : interface.name_;
		getMac(dev->name, interface.mac_);
		for(pcap_addr_t* pa = dev->addresses; pa != nullptr; pa = pa->next) {
			sockaddr* addr = pa->addr;
			sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(addr);
			if(addr != nullptr && addr->sa_family == AF_INET)
				interface.ip_ = ntohl(addr_in->sin_addr.s_addr);

			addr = pa->netmask;
			addr_in = reinterpret_cast<sockaddr_in*>(addr);
			if(addr != nullptr && addr->sa_family == AF_INET) {
				interface.mask_ = ntohl(addr_in->sin_addr.s_addr);
			}
		}
		rep.interfaceList_.push_back(interface);
		dev = dev->next;
		i++;
	}
	pcap_freealldevs(allDevs);

	char buffer[MaxBufferSize];
	int32_t encLen = rep.encode(buffer, MaxBufferSize);
	if (encLen == -1) {
		GTRACE("rep.encode return -1");
		return false;
	}

	int sendLen = ::send(session_->sd_, buffer, encLen, 0);
	if (sendLen == 0 || sendLen == -1) {
		GTRACE("send return %d", sendLen);
		return false;
	}
	return true;
}

//
// ip route show table 0 output
//
// [kali linux]
// default via 10.2.2.1 dev eth0 proto dhcp metric 100 (A)
// 10.2.2.0/24 dev eth0 proto kernel scope link src 10.2.2.3 metric 100 (B)
//
// [android]
// default via 10.2.2.1 dev wlan0  table 1021  proto static (C)
// 10.2.2.0/24 dev wlan0  proto kernel  scope link  src 10.2.2.189 (D)
//
bool GDemonNetwork::processGetRtm(pchar, int32_t) {
	GTRACE("");

	std::string command("ip route show table 0");
	FILE* p = popen(command.data(), "r");
	if (p == nullptr) {
		GTRACE("failed to call %s", command.data());
		return false;
	}

	GetRtmRep rep;
	while (true) {
		char buf[256];
		if (std::fgets(buf, 256, p) == nullptr) break;
		RtmEntry entry;
		if (checkA(buf, &entry))
			rep.rtm_.push_back(entry);
		else if (checkB(buf, &entry))
			rep.rtm_.push_back(entry);
		else if (checkC(buf, &entry))
			rep.rtm_.push_back(entry);
		else if (checkD(buf, &entry))
			rep.rtm_.push_back(entry);
	}
	pclose(p);

	char buffer[MaxBufferSize];
	int32_t encLen = rep.encode(buffer, MaxBufferSize);
	if (encLen == -1) {
		GTRACE("rep.encode return -1");
		return false;
	}

	int sendLen = ::send(session_->sd_, buffer, encLen, 0);
	if (sendLen == 0 || sendLen == -1) {
		GTRACE("send return %d", sendLen);
		return false;
	}
	return true;
}

bool GDemonNetwork::checkA(char* buf, RtmEntry* entry) {
	char gateway[256];
	char intf[256];
	int metric;
	// default via 10.2.2.1 dev eth0 proto dhcp metric 100 (A)
	int res = sscanf(buf, "default via %s dev %s proto dhcp metric %d", gateway, intf, &metric);
	if (res == 3) {
		// GTRACE("checkA %s", buf);
		struct in_addr addr;
		inet_aton(gateway, &addr);
		entry->gateway_ = ntohl(addr.s_addr);
		entry->intfName_ = intf;
		entry->metric_ = metric;
		return true;
	}
	return false;
}

bool GDemonNetwork::checkB(char* buf, RtmEntry* entry) {
	char cidr[256];
	char intf[256];
	char myip[256];
	int metric;
	// 10.2.2.0/24 dev eth0 proto kernel scope link src 10.2.2.3 metric 100 (B)
	int res  = sscanf(buf, "%s dev %s proto kernel scope link src %s metric %d", cidr, intf, myip, &metric);
	if (res == 4) {
		// GTRACE("checkB %s", buf);
		uint32_t dst;
		uint32_t mask;
		if (!decodeCidr(cidr, &dst, &mask)) return false;
		entry->dst_ = dst;
		entry->mask_ = mask;
		entry->intfName_ = intf;
		entry->metric_ = metric;
		return true;
	}
	return false;
}

bool GDemonNetwork::checkC(char* buf, RtmEntry* entry) {
	char gateway[256];
	char intf[256];
	// default via 10.2.2.1 dev wlan0  table 1021  proto static (C)
	int res = sscanf(buf, "default via %s dev %s", gateway, intf);
	if (res == 2) {
		// GTRACE("checkC %s", buf);
		struct in_addr addr;
		inet_aton(gateway, &addr);
		entry->gateway_ = ntohl(addr.s_addr);
		entry->intfName_ = intf;
		return true;
	}
	return false;
}

bool GDemonNetwork::checkD(char* buf, RtmEntry* entry) {
	char cidr[256];
	char intf[256];
	char ip[256];
	// 10.2.2.0/24 dev wlan0  proto kernel  scope link  src 10.2.2.189 (D)
	int res = sscanf(buf, "%s dev %s proto kernel scope link src %s", cidr, intf, ip);
	if (res == 3) {
		// GTRACE("checkD %s", buf);
		uint32_t dst;
		uint32_t mask;
		if (!decodeCidr(cidr, &dst, &mask)) return false;
		entry->dst_ = dst;
		entry->mask_ = mask;
		entry->intfName_ = intf;
		return true;
	}
	return false;
}

bool GDemonNetwork::decodeCidr(std::string cidr, uint32_t* dst, uint32_t* mask) {
	size_t found = cidr.find("/");
	if (found == std::string::npos) return false;
	std::string dstStr = cidr.substr(0, found);
	struct in_addr addr;
	inet_aton(dstStr.data(), &addr);
	*dst = ntohl(addr.s_addr);
	std::string maskStr = cidr.substr(found + 1);
	*mask = numberToMask(std::stoi(maskStr.data()));
	return true;
}

uint32_t GDemonNetwork::numberToMask(int number) {
	uint32_t res = 0;
	for (int i = 0; i < number; i++) {
		res = (res >> 1);
		res |= 0x80000000;
	}
	return res;
}

// ----------------------------------------------------------------------------
// GDemonPcap
// ----------------------------------------------------------------------------
GDemonPcap::GDemonPcap(GDemonSession* session) : session_(session) {
}

GDemonPcap::~GDemonPcap() {
	close();
}

GDemon::PcapOpenRep GDemonPcap::open(PcapOpenReq req) {
	PcapOpenRep rep;
	if (pcap_ != nullptr) {
		rep.errBuf_ = "pcap is already opened";
	} else {
		char errBuf[PCAP_ERRBUF_SIZE];
		pcap_ = pcap_open_live(req.intfName_.data(), req.snaplen_, req.flags_, req.readTimeout_, errBuf);
		if (pcap_ == nullptr) {
			rep.errBuf_ = errBuf;
			GTRACE("pcap_open_live return null %s", rep.errBuf_.data());
		} else {
			rep.result_ = true;
			if (req.filter_ != "") {
				u_int uNetMask;
				bpf_program code;

				uNetMask = 0xFFFFFFFF;
				if (pcap_compile(pcap_, &code, req.filter_.data(),  1, uNetMask) < 0) {
					rep.result_ = false;
					rep.errBuf_ = pcap_geterr(pcap_);
					GTRACE("error in pcap_compile(%s)", rep.errBuf_.data()) ;
				} else
				if (pcap_setfilter(pcap_, &code) < 0) {
					rep.result_ = false;
					rep.errBuf_ = pcap_geterr(pcap_);
					GTRACE("error in pcap_setfilter(%s)", rep.errBuf_.data()) ;
				}
			}
			rep.dataLink_ = pcap_datalink(pcap_);
			if (req.captureThread_) {
				active_ = true;
				thread_ = new std::thread(GDemonPcap::_run, this, req.waitTimeout_);
			}
		}
	}
	return rep;
}

void GDemonPcap::close() {
	if (thread_ != nullptr) {
		active_ = false;
		thread_->join();
		delete thread_;
		thread_ = nullptr;
	}

	if (pcap_ != nullptr) {
		pcap_close(pcap_);
		pcap_ = nullptr;
	}
}

void GDemonPcap::_run(GDemonPcap* pcap, int waitTimeout) {
	pcap->run(waitTimeout);
}

void GDemonPcap::run(int waitTimeout) {
	PcapRead read;
	while (active_) {
		pcap_pkthdr* pktHdr;
		const u_char* data;
		int i = pcap_next_ex(pcap_, &pktHdr, &data);
		switch (i) {
			case -2:
			case -1:
				GTRACE("pcap_next_ex return %d", i);
				active_ = false;
				return;
			case 0:
				if (waitTimeout != 0)
					std::this_thread::sleep_for(std::chrono::microseconds(waitTimeout));
				continue;
			default:
				break;
		}

		read.pktHdr_ = *pktHdr;
		read.data_ = puchar(data);

		char buffer[MaxBufferSize];
		int32_t encLen = read.encode(buffer, MaxBufferSize);
		if (encLen == -1) {
			GTRACE("rep.encode return -1");
			return;
		}

		int sendLen = ::send(session_->sd_, buffer, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return;
		}
	}
}

bool GDemonPcap::processPcapOpen(pchar buf, int32_t size) {
	GTRACE("");

	PcapOpenReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}

	PcapOpenRep rep = open(req);

	char buffer[MaxBufferSize];
	int32_t encLen = rep.encode(buffer, MaxBufferSize);
	if (encLen == -1) {
		GTRACE("rep.encode return -1");
		return false;
	}

	int sendLen = ::send(session_->sd_, buffer, encLen, 0);
	if (sendLen == 0 || sendLen == -1) {
		GTRACE("send return %d", sendLen);
	}
	return true;
}

bool GDemonPcap::processPcapClose(pchar buf, int32_t size) {
	GTRACE("");

	PcapCloseReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}

	close();
	return false;
}

bool GDemonPcap::processPcapWrite(pchar buf, int32_t size) {
	GTRACE("");

	PcapWrite write;
	int32_t decLen = write.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return -1");
		return false;
	}

	int i = pcap_sendpacket(pcap_, write.data_, write.size_);
	if (i != 0) {
		char* e = pcap_geterr(pcap_);
		if (e == nullptr) e = pchar("unknown");
		GTRACE("pcap_sendpacket return %d(%s) length=%d", i, e , write.size_);
	}

	return true;
}

