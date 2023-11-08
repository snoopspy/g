#include "gdemonclient.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GDemonClientMapKey
// ----------------------------------------------------------------------------
struct GDemonClientMapKey {
	GDemonClientMapKey(std::string ip, uint16_t port) : ip_(ip), port_(port) {}
	std::string ip_;
	uint16_t port_;
	bool operator<(const GDemonClientMapKey &r) const {
		if (ip_ < r.ip_) return true;
		if (ip_ > r.ip_) return false;
		if (port_ < r.port_) return true;
		if (port_ > r.port_) return false;
		return false;
	}
};

// ----------------------------------------------------------------------------
// GDemonClientMap
// ----------------------------------------------------------------------------
struct GDemonClientMap : std::map<GDemonClientMapKey, GDemonClient*> {
	friend struct GDemonClient;

protected: // singleton
	virtual ~GDemonClientMap() {
		for (GDemonClientMap::iterator it = begin(); it != end(); it++) {
			delete it->second;
		}
	}
};

// ----------------------------------------------------------------------------
// GDemonClient
// ----------------------------------------------------------------------------
GDemonClient::GDemonClient(std::string ip, uint16_t port) : ip_(ip), port_(port) {
	recvBuf_ = new char[MaxBufSize];
	sendBuf_ = new char[MaxBufSize];
	pktBuf_ = new char[MaxBufSize];

	connect();
}

GDemonClient::~GDemonClient() {
	disconnect();

	if (recvBuf_ != nullptr) {
		delete[] recvBuf_;
		recvBuf_ = nullptr;
	}

	if (sendBuf_ != nullptr) {
		delete[] sendBuf_;
		sendBuf_ = nullptr;
	}

	if (pktBuf_ != nullptr) {
		delete[] pktBuf_;
		pktBuf_ = nullptr;
	}
}

bool GDemonClient::connect() {
	if (sd_ != 0) {
		qDebug() << "already connected";
		return true;
	}

	sd_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sd_ == -1) {
		error_ = strerror(errno);
		qWarning() << strerror(errno);
		sd_ = 0;
		return false;
	}

#ifdef __linux__
	in_addr_t ip_addr;
#endif // __linux__
#ifdef WIN32
	unsigned ip_addr;
#endif // WIN32
	ip_addr = inet_addr(ip_.data());

	struct sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port_);
	addr_in.sin_addr.s_addr = ip_addr;
	memset(&addr_in.sin_zero, 0, sizeof(addr_in.sin_zero));

	bool connected = false;
	for (int i = 0; i < 10 ; i++) { // 5 seconds
		int res = ::connect(sd_, (struct sockaddr*)&addr_in, sizeof(struct sockaddr_in));
		if (res != -1) {
			connected = true;
			break;
		}
		qWarning() << strerror(errno);
		usleep(500000); // 0.5 sec
	}

	if (!connected) {
		::shutdown(sd_, 2/* SHUT_RDWR SD_BOTH*/);
		::close(sd_);
		sd_ = 0;
		error_ = qPrintable(QString("can not connect to ssdemon %1 %2").arg(ip_.data()).arg(port_));
		qCritical() << error_.data();
		return false;
	}
	return true;
}

bool GDemonClient::disconnect() {
	if (sd_ != 0) {
		::shutdown(sd_, 2/* SHUT_RDWR SD_BOTH*/);
		::close(sd_);
		sd_ = 0;
	}
	return true;
}

GDemon::CmdExecuteRes GDemonClient::cmdExecute(std::string command) {
	CmdExecuteRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	CmdExecuteReq req;

	req.command_ = command;
	int32_t encLen = req.encode(sendBuf_, MaxBufSize);
	if (encLen == -1) {
		error_ = "req.encode return -1";
		return res;
	}

	int sendLen = ::send(sd_, sendBuf_, encLen, 0);
	if (sendLen == 0 || sendLen == -1) {
		error_ = qPrintable(QString("send return %1").arg(sendLen));
		qWarning() << error_.data();
		return res;
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		qWarning() << "res.decode return -1";
	}

	if (!res.result_)
		error_ = res.error_;

	return res;
}

GDemon::CmdStartRes GDemonClient::cmdStart(std::string command) {
	CmdStartRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	CmdStartReq req;

	req.command_ = command;
	int32_t encLen = req.encode(sendBuf_, MaxBufSize);
	if (encLen == -1) {
		error_ = "req.encode return -1";
		return res;
	}

	int sendLen = ::send(sd_, sendBuf_, encLen, 0);
	if (sendLen == 0 || sendLen == -1) {
		error_ = qPrintable(QString("send return %1").arg(sendLen));
		qWarning() << error_.data();
		return res;
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		qWarning() << "res.decode return -1";
	}

	if (res.pid_ == 0)
		error_ = res.error_;

	return res;
}

GDemon::CmdStopRes GDemonClient::cmdStop(uint64_t pid) {
	CmdStopRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	CmdStopReq req;

	req.pid_ = pid;
	int32_t encLen = req.encode(sendBuf_, MaxBufSize);
	if (encLen == -1) {
		error_ = "req.encode return -1";
		return res;
	}

	int sendLen = ::send(sd_, sendBuf_, encLen, 0);
	if (sendLen == 0 || sendLen == -1) {
		error_ = qPrintable(QString("send return %1").arg(sendLen));
		qWarning() << error_.data();
		return res;
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		qWarning() << "res.decode return -1";
	}

	if (!res.result_)
		error_ = res.error_;

	return res;
}

GDemon::CmdStartDetachedRes GDemonClient::cmdStartDetached(std::string command) {
	CmdStartDetachedRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	CmdStartDetachedReq req;
	req.command_ = command;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			return res;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return res;
		}
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		qWarning() << "res.decode return -1";
	}

	if (!res.result_)
		error_ = res.error_;

	return res;

}

GDemon::GetInterfaceListRes GDemonClient::getInterfaceList() {
	GetInterfaceListRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	GetInterfaceListReq req;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			qWarning() << error_.data();
			return res;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return res;
		}
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		error_ = "res.decode return -1";
		qWarning() << error_.data();
	}

	return res;
}

GDemon::GetRtmRes GDemonClient::getRtm() {
	GetRtmRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	GetRtmReq req;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			qWarning() << error_.data();
			return res;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return res;
		}
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		error_ = "res.decode return -1";
		qWarning() << error_.data();
	}

	return res;
}

GDemon::PcapOpenRes GDemonClient::pcapOpen(std::string client, std::string filter, std::string intfName, int32_t snapLen, int32_t flags, int32_t readTimeout, int32_t waitTimeout, bool nonBlock, bool captureThread) {
	GDemon::PcapOpenRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	PcapOpenReq req;
	req.client_ = client;
	req.filter_ = filter;
	req.intfName_ = intfName;
	req.snaplen_ = snapLen;
	req.flags_ = flags;
	req.readTimeout_ = readTimeout;
	req.waitTimeout_ = waitTimeout;
	req.nonBlock_ = nonBlock;
	req.captureThread_ = captureThread;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			return res;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return res;
		}
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		qWarning() << "res.decode return -1";
	}

	if (!res.result_)
		error_ = res.errBuf_;

	return res;
}

void GDemonClient::pcapClose() {
	PcapCloseReq req;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			qWarning() << error_.data();
			return;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			// qWarning() << error_.data(); // do not log for disconnected state
			return;
		}
	}
	disconnect();
}

GDemon::PcapRead GDemonClient::pcapRead() {
	GDemon::PcapRead read;

	Header* header = GDemon::PHeader(pktBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return read;
	}

	if (!recvAll(sd_, pktBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return read;
	}

	int32_t decLen = read.decode(pktBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		error_ = "res.decode return -1";
		qWarning() << error_.data();
		read.data_ = nullptr;
	}

	return read;
}

bool GDemonClient::pcapWrite(PcapWrite write) {
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = write.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			qWarning() << error_.data();
			return false;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return false;
		}
	}
	return true;
}

GDemon::NfOpenRes GDemonClient::nfOpen(std::string client, uint16_t queueNum, uint32_t waitTimeout, bool nonBlock) {
	GDemon::NfOpenRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	NfOpenReq req;
	req.client_ = client;
	req.queueNum_ = queueNum;
	req.waitTimeout_ = waitTimeout;
	req.nonBlock_ = nonBlock;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			return res;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return res;
		}
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		qWarning() << "res.decode return -1";
	}

	if (!res.result_)
		error_ = res.errBuf_;

	return res;
}

void GDemonClient::nfClose() {
	NfCloseReq req;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			qWarning() << error_.data();
			return;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			// qWarning() << error_.data(); // do not log for disconnected state
			return;
		}
	}
	disconnect();
}

GDemon::NfRead GDemonClient::nfRead() {
	GDemon::NfRead read;

	Header* header = GDemon::PHeader(pktBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return read;
	}

	if (!recvAll(sd_, pktBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return read;
	}

	int32_t decLen = read.decode(pktBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		error_ = "res.decode return -1";
		qWarning() << error_.data();
		read.data_ = nullptr;
	}

	return read;
}

bool GDemonClient::nfVerdict(GDemon::NfVerdict verdict) {
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = verdict.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			qWarning() << error_.data();
			return false;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return false;
		}
	}
	return true;
}

GDemon::RiOpenRes GDemonClient::riOpen() {
	GDemon::RiOpenRes res;

	if (sd_ == 0) {
		error_ = "not connected state";
		qWarning() << error_.data();
		return res;
	}

	RiOpenReq req;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			return res;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return res;
		}
	}

	Header* header = GDemon::PHeader(recvBuf_);
	if (!header->recv(sd_)) {
		error_ = "recvAll(header) return false";
		return res;
	}

	if (!recvAll(sd_, recvBuf_ + sizeof(Header), header->len_)) {
		error_ = "recvAll(body) return false";
		qWarning() << error_.data();
		return res;
	}

	int32_t decLen = res.decode(recvBuf_, sizeof(Header) + header->len_);
	if (decLen == -1) {
		qWarning() << "res.decode return -1";
	}

	if (!res.result_)
		error_ = res.errBuf_;

	return res;
}

void GDemonClient::riClose() {
	RiCloseReq req;
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = req.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			qWarning() << error_.data();
			return;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			// qWarning() << error_.data(); // do not log for disconnected state
			return;
		}
	}
	disconnect();
}

bool GDemonClient::riWrite(RiWrite write) {
	{
		GSpinLockGuard guard(sendBufLock_);
		int32_t encLen = write.encode(sendBuf_, MaxBufSize);
		if (encLen == -1) {
			error_ = "req.encode return -1";
			qWarning() << error_.data();
			return false;
		}

		int sendLen = ::send(sd_, sendBuf_, encLen, 0);
		if (sendLen == 0 || sendLen == -1) {
			error_ = qPrintable(QString("send return %1").arg(sendLen));
			qWarning() << error_.data();
			return false;
		}
	}
	return true;
}

GDemonClient& GDemonClient::instance(std::string ip, uint16_t port) {
	static GDemonClientMap map;
	GDemonClientMapKey key(ip, port);
	GDemonClientMap::iterator it = map.find(key);
	if (it == map.end()) {
		map.insert({key, new GDemonClient(ip, port)});
		it = map.find(key);
	}
	return *(it->second);
}

