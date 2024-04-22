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

	struct sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = INADDR_ANY;
	addr_in.sin_port = htons(port);

	res = ::bind(accept_, (struct sockaddr*)&addr_in, sizeof(struct sockaddr_in));
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
		struct sockaddr_in addr_in;
		socklen_t len = sizeof(addr_in);
		int new_sd = ::accept(accept_, (struct sockaddr*)&addr_in, &len);
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
		::shutdown(accept_, SHUT_RDWR);
		::close(accept_);
		accept_ = 0;
	}

	sessions_.lock();
	for (GDemonSession* session: sessions_) {
		::shutdown(session->sd_, SHUT_RDWR);
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
		usleep(1000);
	}
}

// ----------------------------------------------------------------------------
// GDemonSession
// ----------------------------------------------------------------------------
GDemonSession::GDemonSession(GDemonServer* server) : server_(server) {
	recvBuf_ = new char[MaxBufSize];
	sendBuf_ = new char[MaxBufSize];
}

GDemonSession::~GDemonSession() {
	if (sd_ != 0) {
		::shutdown(sd_, SHUT_RDWR);
		::close(sd_);
		sd_ = 0;
	}

	if (command_ == nullptr) {
		delete command_;
		command_ = nullptr;
	}

	if (network_ != nullptr) {
		delete network_;
		network_ = nullptr;
	}

	if (pcap_ != nullptr) {
		delete pcap_;
		pcap_ = nullptr;
	}

	if (nf_ != nullptr) {
		delete nf_;
		nf_ = nullptr;
	}

	if (recvBuf_ != nullptr) {
		delete[] recvBuf_;
		recvBuf_ = nullptr;
	};

	if (sendBuf_ != nullptr) {
		delete[] sendBuf_;
		sendBuf_ = nullptr;
	};
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
		Header* header = GDemon::PHeader(recvBuf_);

		if (header->recv(sd_) == -1)
			break;

		if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
			GTRACE("recvAll(%d) return false", header->len_);
			break;
		}

		pchar buf = recvBuf_;
		int size = header->len_ + sizeof(Header);
		switch (header->cmd_) {
			case CmdCmdExecute:
				if (command_ == nullptr) command_ = new GDemonCommand(this);
				active = command_->processCmdExecute(buf, size);
				break;
			case CmdCmdStart:
				if (command_ == nullptr) command_ = new GDemonCommand(this);
				active = command_->processCmdStart(buf, size);
				break;
			case CmdCmdStop:
				if (command_ == nullptr) command_ = new GDemonCommand(this);
				active = command_->processCmdStop(buf, size);
				break;
			case CmdCmdStartDetached:
				if (command_ == nullptr) command_ = new GDemonCommand(this);
				active = command_->processCmdStartDetached(buf, size);
				break;
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
			case CmdNfOpen:
				if (nf_ == nullptr) nf_ = new GDemonNetFilter(this);
				active = nf_->processNfOpen(buf, size);
				break;
			case CmdNfClose:
				if (nf_ == nullptr) nf_ = new GDemonNetFilter(this);
				active = nf_->processNfClose(buf, size);
				break;
			case CmdNfVerdict:
				if (nf_ == nullptr) nf_ = new GDemonNetFilter(this);
				active = nf_->processNfVerdict(buf, size);
				break;
			case CmdRiOpen:
				if (ri_ == nullptr) ri_ = new GDemonRawIp(this);
				active = ri_->processRiOpen(buf, size);
				break;
			case CmdRiClose:
				if (ri_ == nullptr) ri_ = new GDemonRawIp(this);
				active = ri_->processRiClose(buf, size);
				break;
			case CmdRiWrite:
				if (ri_ == nullptr) ri_ = new GDemonRawIp(this);
				active = ri_->processRiWrite(buf, size);
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
// GDemonCommand
// ----------------------------------------------------------------------------
GDemonCommand::GDemonCommand(GDemonSession* session) : session_(session) {
}

GDemonCommand::~GDemonCommand() {
}

bool GDemonCommand::processCmdExecute(pchar buf, int32_t size) {
	CmdExecuteReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}

	std::string command = req.command_;
	std::string error;
	bool result = GProcess::execute(command, error);
	GTRACE("%s return %s", command.data(), result ? "true" : "false");

	CmdExecuteRes res;
	res.result_ = result;
	if (!res.result_) { // fail
		res.error_ = error;
		GTRACE("%s", res.error_.data());
	}

	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
	}
	return true;
}

bool GDemonCommand::processCmdStart(pchar buf, int32_t size) {
	CmdStartReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}

	std::string command = req.command_;
	std::string error;
	pid_t pid = GProcess::start(command, error);
	GTRACE("%s return %d", command.data(), pid);

	CmdStartRes res;
	res.pid_ = pid;
	if (res.pid_ <= 0) { // fail
		res.error_ = error;
		GTRACE("%s", res.error_.data());
	}

	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
	}
	return true;
}

bool GDemonCommand::processCmdStop(pchar buf, int32_t size) {
	CmdStopReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}

	pid_t pid = req.pid_;
	std::string error;
	bool result = GProcess::stop(pid, error);
	GTRACE("GProcess:stop(%d) return %s", pid, result ? "true" : "false");

	CmdStopRes res;
	res.result_ = (result);
	if (!res.result_) {
		res.error_ = error;
		GTRACE("%s", res.error_.data());
	}

	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
	}
	return true;
}

bool GDemonCommand::processCmdStartDetached(pchar buf, int32_t size) {
	CmdStartDetachedReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}

	std::string command = req.command_ + " &";
	GTRACE("%s", command.data());
	int result = system(command.data());

	CmdStartDetachedRes res;
	res.result_ = (result == 0);
	if (!res.result_) {
		res.error_ = std::string("system(") + command + ") return " + std::to_string(result) + " errno=" + std::to_string(errno);
		GTRACE("%s", res.error_.data());
	}

	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
	}
	return true;
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
	GetInterfaceListRes res;
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
		res.interfaceList_.push_back(interface);
		dev = dev->next;
		i++;
	}
	pcap_freealldevs(allDevs);

	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
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

	GetRtmRes res;
	while (true) {
		char buf[256];
		if (std::fgets(buf, 256, p) == nullptr) break;
		RtmEntry entry;
		if (checkA(buf, &entry))
			res.rtm_.push_back(entry);
		else if (checkB(buf, &entry))
			res.rtm_.push_back(entry);
		else if (checkC(buf, &entry))
			res.rtm_.push_back(entry);
		else if (checkD(buf, &entry))
			res.rtm_.push_back(entry);
	}
	pclose(p);

	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
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

GDemon::PcapOpenRes GDemonPcap::open(PcapOpenReq req) {
	PcapOpenRes res;

	if (pcap_ != nullptr) {
		res.errBuf_ = "pcap is already opened";
		return res;
	}
	waitTimeout_ = req.waitTimeout_;
	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_live(req.intfName_.data(), req.snaplen_, req.flags_, req.readTimeout_, errBuf);
	if (pcap_ == nullptr) {
		res.errBuf_ = errBuf;
		GTRACE("pcap_open_live return null %s", res.errBuf_.data());
		return res;
	}

	if (req.filter_ != "") {
		u_int uNetMask;
		bpf_program code;

		uNetMask = 0xFFFFFFFF;
		if (pcap_compile(pcap_, &code, req.filter_.data(),  1, uNetMask) < 0) {
			res.errBuf_ = pcap_geterr(pcap_);
			GTRACE("error in pcap_compile(%s)", res.errBuf_.data());
			close();
			return res;
		}
		if (pcap_setfilter(pcap_, &code) < 0) {
			res.errBuf_ = pcap_geterr(pcap_);
			GTRACE("error in pcap_setfilter(%s)", res.errBuf_.data());
			close();
			return res;
		}
	}

	res.result_ = true;
	res.dataLink_ = pcap_datalink(pcap_);

	return res;
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

void GDemonPcap::run() {
	GTRACE("GDemonPcap beg");

	PcapRead read;

	while (active_) {
		pcap_pkthdr* pktHdr;
		const u_char* data;
		int i = pcap_next_ex(pcap_, &pktHdr, &data);
		switch (i) {
			case PCAP_ERROR_BREAK: // if EOF was reached reading from an offline capture
			case PCAP_ERROR: { // if an error occurred
				char* e = pcap_geterr(pcap_);
				if (e == nullptr || strlen(e) == 0)
					e = const_cast<char*>("unknown");
				GTRACE("pcap_next_ex return %d error=%s", i, e);
				active_ = false;
				break;
			}
			case 0:
				if (waitTimeout_ != 0)
					std::this_thread::sleep_for(std::chrono::milliseconds(waitTimeout_));
				continue;
			default:
				break;
		}

		read.pktHdr_.tv_sec_ = (uint64_t)pktHdr->ts.tv_sec;
		read.pktHdr_.tv_usec_ = (uint64_t)pktHdr->ts.tv_usec;
		read.pktHdr_.caplen_ = (uint32_t)pktHdr->caplen;
		read.pktHdr_.len_ = (uint32_t)pktHdr->len;
		read.data_ = puchar(data);

		{
			GSpinLockGuard guard(session_->sendBufLock_);
			int32_t encLen = read.encode(session_->sendBuf_, MaxBufSize);
			if (encLen == -1) {
				GTRACE("res.encode return -1");
				break;
			}

			int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
			if (sendLen == 0 || sendLen == -1) {
				GTRACE("send return %d", sendLen);
				break;
			}
		}
	}

	GTRACE("GDemonPcap end");
	// disconnect connection
	::shutdown(session_->sd_, SHUT_RDWR);
	::close(session_->sd_);
}

bool GDemonPcap::processPcapOpen(pchar buf, int32_t size) {
	PcapOpenReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}
	client_ = req.client_;
	GTRACE("%s", client_.data());

	PcapOpenRes res = open(req);
	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
	}

	if (res.result_) {
		active_ = true;
		if (req.captureThread_)
			thread_ = new std::thread(&GDemonPcap::run, this);
	}

	return true;
}

bool GDemonPcap::processPcapClose(pchar buf, int32_t size) {
	GTRACE("%s", client_.data());

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

// ----------------------------------------------------------------------------
// GDemonNetFilter
// ----------------------------------------------------------------------------
GDemonNetFilter::GDemonNetFilter(GDemonSession* session) : session_(session) {
}

GDemonNetFilter::~GDemonNetFilter() {
	close();
}

#include <fcntl.h> // for F_GETFL
GDemon::NfOpenRes GDemonNetFilter::open(NfOpenReq req) {
	NfOpenRes res;

	if (h_ != nullptr) {
		res.errBuf_ = "netfilter is already opened";
		return res;
	}

	// opening library handle
	h_ = nfq_open();
	if (!h_) {
		res.errBuf_ = "nfq_open return null";
		GTRACE("%s", res.errBuf_.data());
		close();
		return res;
	}

	// unbinding existing nf_queue handler for AF_INET (if any)
	if (nfq_unbind_pf(h_, AF_INET) < 0) {
		res.errBuf_ = "error during nfq_unbind_pf()";
		GTRACE("%s", res.errBuf_.data());
		close();
		return res;
	}

	// binding nfnetlink_queue as nf_queue handler for AF_INET
	if (nfq_bind_pf(h_, AF_INET) < 0) {
		res.errBuf_ = "error during nfq_bind_pf()";
		GTRACE("%s", res.errBuf_.data());
		close();
		return res;
	}

	// binding this socket to queue
	qh_ = nfq_create_queue(h_, req.queueNum_, &_callback, this);
	if (!qh_) {
		res.errBuf_ = "error during nfq_create_queue()";
		GTRACE("%s", res.errBuf_.data());
		close();
		return res;
	}

	// setting copy_packet mode
	if (nfq_set_mode(qh_, NFQNL_COPY_PACKET, 0xffff) < 0) {
		res.errBuf_ = "can't set packet_copy mode";
		GTRACE("%s", res.errBuf_.data());
		close();
		return res;
	}

	fd_ = nfq_fd(h_);
	if (req.nonBlock_) {
		int flags = fcntl(fd_, F_GETFL);
		fcntl(fd_, F_SETFL,flags| O_NONBLOCK);
	}

	nfRecvBuf_ = new char[MaxBufSize];

	id_ = 0;
	packet_ = nullptr;

	res.result_ = true;

	return res;
}

void GDemonNetFilter::close() {
	if (fd_ != 0) {
		GTRACE("bef call ::shutdown"); // gilgil temp 2016.09.25
		::shutdown(fd_, SHUT_RDWR);
		GTRACE("aft call ::shutdown"); // gilgil temp 2016.09.25

		GTRACE("bef call ::close"); // gilgil temp 2016.09.25
		::close(fd_);
		GTRACE("aft call ::close"); // gilgil temp 2016.09.25
		fd_ = 0;
	}

	if (thread_ != nullptr) {
		active_ = false;
		GTRACE("bef thread_->join"); // gilgil temp 2021.06.27
		thread_->join();
		GTRACE("aft thread_->join"); // gilgil temp 2021.06.27
		delete thread_;
		thread_ = nullptr;
	}

	if (qh_ != nullptr) {
		GTRACE("bef call nfq_destroy_queue"); // gilgil temp 2016.09.25
		nfq_destroy_queue(qh_);
		GTRACE("aft call nfq_destroy_queue"); // gilgil temp 2016.09.25
		qh_ = nullptr;
	}

	if (h_ != nullptr) {
#ifdef INSANE
		// normally, applications SHOULD NOT issue this command, since
		// it detaches other programs/sockets from AF_INET, too !
		// unbinding from AF_INET
		nfq_unbind_pf(h, AF_INET);
#endif // INSANE

		// closing library handle
		GTRACE("bef call nfq_close"); // gilgil temp 2016.09.25
		nfq_close(h_);
		GTRACE("aft call nfq_close"); // gilgil temp 2016.09.25
		h_ = nullptr;
	}

	if (nfRecvBuf_ != nullptr) {
		delete[] nfRecvBuf_;
		nfRecvBuf_ = nullptr;
	}
}

void GDemonNetFilter::run() {
	GTRACE("GDemonNetFilter beg");
	while (active_) {
		// GTRACE("bef call recv"); // gilgil temp 2016.09.27
		int res = int(::recv(fd_, nfRecvBuf_, MaxBufSize, 0));
		// GTRACE("aft call recv %d", res); // gilgil temp 2016.09.27
		if (res < 0) {
			if (errno == EWOULDBLOCK) {
				if (waitTimeout_ != 0)
					std::this_thread::sleep_for(std::chrono::milliseconds(waitTimeout_));
				continue;
			}
			if (errno == ENOBUFS) {
				GTRACE("losing packets!");
				continue;
			}
			GTRACE("recv return %d errno=%d", res, errno);
			break;
		}
		nfq_handle_packet(h_, pchar(nfRecvBuf_), res);
	}

	GTRACE("GDemonNetFilter end");
	// disconnect connection
	::shutdown(session_->sd_, SHUT_RDWR);
	::close(session_->sd_);
}


bool GDemonNetFilter::processNfOpen(pchar buf, int32_t size) {
	NfOpenReq req;

	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}
	client_ = req.client_;
	GTRACE("%s", client_.data());

	NfOpenRes res = open(req);
	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
	}

	if (res.result_) {
		active_ = true;
		thread_ = new std::thread(&GDemonNetFilter::run, this);
	}

	return true;
}

bool GDemonNetFilter::processNfClose(pchar buf, int32_t size) {
	GTRACE("%s", client_.data());

	NfCloseReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}

	close();
	return false;
}

bool GDemonNetFilter::processNfVerdict(pchar buf, int32_t size) {
	NfVerdict verdict;
	int32_t decLen = verdict.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return -1");
		return false;
	}

	nfq_set_verdict2(qh_, verdict.id_, verdict.acceptVerdict_, verdict.mark_, verdict.size_, verdict.data_);
	// GTRACE("nfq_set_verdict2 return %d", res); // gilgil temp 2021.06.28

	return true;
}

int GDemonNetFilter::_callback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfad, void* data) {
	(void)qh;
	(void)nfmsg;

	NfRead read;

	timeval ts;
	gettimeofday(&ts, nullptr);
	unsigned char* packet;
	int res = nfq_get_payload(nfad, &packet);
	if (res == -1) {
		GTRACE("nfq_get_payload return -1");
		return -1;
	}

	read.pktHdr_.tv_sec_ = (uint64_t)ts.tv_sec;
	read.pktHdr_.tv_usec_ = (uint64_t)ts.tv_usec;
	read.pktHdr_.len_ = (uint32_t)res;
	read.id_ = 0;
	struct nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfad);
	if (ph != nullptr)
		read.id_ = ntohl(ph->packet_id);
	read.data_ = puchar(packet);

	GDemonNetFilter* nf = reinterpret_cast<GDemonNetFilter*>(data);
	{
		GSpinLockGuard guard(nf->session_->sendBufLock_);
		int32_t encLen = read.encode(nf->session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return -1;
		}

		int sendLen = ::send(nf->session_->sd_, nf->session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return -1;
		}
	}

	return 0; // gilgil temp 2021.06.27
}

// ----------------------------------------------------------------------------
// GDemonRawIp
// ----------------------------------------------------------------------------
GDemonRawIp::GDemonRawIp(GDemonSession* session) : session_(session) {
}

GDemonRawIp::~GDemonRawIp() {
	close();
}

GDemon::RiOpenRes GDemonRawIp::open(RiOpenReq req) {
	(void)req;

	RiOpenRes res;

	if (sd_ != 0) {
		res.errBuf_ = "sd_ is already opened";
		return res;
	}

	sd_ = ::socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sd_ == -1) {
		res.errBuf_ = std::string("socket return -1 ") + strerror(errno);
		GTRACE("%s", res.errBuf_.data());
		return res;
	}

	int one = 1;
	int r = ::setsockopt(sd_, IPPROTO_IP, IP_HDRINCL, pchar(&one), sizeof(one));
	if (r < 0) {
		res.errBuf_ = std::string("setsockopt return ") + std::to_string(r) + " " + strerror(errno);
		GTRACE("%s", res.errBuf_.data());
		close();
		return res;
	}

	intfName_ = req.intfName_;
	if (intfName_ != "") {
		r = ::setsockopt(sd_, SOL_SOCKET, SO_BINDTODEVICE, intfName_.data(), intfName_.size());
		if (r < 0) {
			res.errBuf_ = std::string("setsockopt(SO_BINDTODEVICE) return ") + std::to_string(r) + " " + strerror(errno);
			GTRACE("%s", res.errBuf_.data());
			close();
			return res;
		}
	}

	res.result_ = true;

	return res;
}

void GDemonRawIp::close() {
	if (sd_ != 0) {
		::close(sd_);
		sd_ = 0;
	}
}

bool GDemonRawIp::processRiOpen(pchar buf, int32_t size) {
	RiOpenReq req;

	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}
	client_ = req.client_;
	GTRACE("%s", client_.data());

	RiOpenRes res = open(req);
	{
		GSpinLockGuard guard(session_->sendBufLock_);
		int32_t encLen = res.encode(session_->sendBuf_, MaxBufSize);
		if (encLen == -1) {
			GTRACE("res.encode return -1");
			return false;
		}

		int sendLen = ::send(session_->sd_, session_->sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			GTRACE("send return %d", sendLen);
			return false;
		}
	}

	if (res.result_)
		active_ = true;

	return true;
}

bool GDemonRawIp::processRiClose(pchar buf, int32_t size) {
	RiCloseReq req;
	int32_t decLen = req.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return =1");
		return false;
	}

	close();
	return false;
}

bool GDemonRawIp::processRiWrite(pchar buf, int32_t size) {
	RiWrite write;
	int32_t decLen = write.decode(buf, size);
	if (decLen == -1) {
		GTRACE("req.decode return -1");
		return false;
	}

	struct sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	uint32_t dip = *reinterpret_cast<uint32_t*>(write.data_ + 16); // dip index
	addr_in.sin_addr.s_addr = dip; // network byte order

	int res = ::sendto(sd_, write.data_, write.size_, 0, (struct sockaddr*)&addr_in, sizeof(struct sockaddr_in));
	if (res < 0) {
		GTRACE("sendto return %d(%s) buf len=%d", res, strerror(errno), write.len_);
		return false;
	}

	return true;
}
