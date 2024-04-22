#include "gnetfilter.h"

#include <fcntl.h> // for F_GETFL

// ----------------------------------------------------------------------------
// GNetFilter
// ----------------------------------------------------------------------------
GNetFilter::GNetFilter(QObject* parent) : GCapture(parent) {
	command_.openCommands_.clear();
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{
#ifdef _DEBUG
		"su -c \"iptables -F\"",
#endif // _DEBUG
		"su -c \"iptables -A OUTPUT -o lo -j ACCEPT\"",
		"su -c \"iptables -A INPUT -i lo -j ACCEPT\"",
		"su -c \"iptables -A OUTPUT -j NFQUEUE\"",
		"su -c \"iptables -A INPUT -j NFQUEUE\""
	}));

	command_.closeCommands_.clear();
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{
		"su -c \"iptables -D INPUT -j NFQUEUE\"",
		"su -c \"iptables -D OUTPUT -j NFQUEUE\"",
		"su -c \"iptables -D INPUT -i lo -j ACCEPT\"",
		"su -c \"iptables -D OUTPUT -o lo -j ACCEPT\""
	}));

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	cb_ = &_callback;
#endif
}

GNetFilter::~GNetFilter() {
	close();
}

#ifdef Q_OS_ANDROID
bool GNetFilter::doOpen() {
	if (!enabled_) return true;

	if (objectName() != "") command_.setObjectName(objectName() + ".command_");
	if (!command_.open()) {
		err = command_.err;
		return false;
	}

	demonClient_ = new GDemonClient("127.0.0.4", GDemon::DefaultPort);
	GDemon::NfOpenRes res = demonClient_->nfOpen(std::string(qPrintable(objectName())), queueNum_, waitTimeout_, nonBlock_);
	if (!res.result_) {
		SET_ERR(GErr::Fail, demonClient_->error_.data());
		delete demonClient_; demonClient_ = nullptr;
		return false;
	}

	return GCapture::doOpen();
}

bool GNetFilter::doClose() {
	if (!enabled_) return true;

	if (demonClient_ != nullptr)
		demonClient_->nfClose();

	GCapture::doClose();

	if (demonClient_ != nullptr) {
		delete demonClient_;
		demonClient_ = nullptr;
	}

	command_.close();

	return true;
}

GPacket::Result GNetFilter::read(GPacket* packet) {
	packet->clear();

	GDemon::NfRead read = demonClient_->nfRead();
	if (read.data_ == nullptr) {
		SET_ERR(GErr::ReadFailed, "read fail");
		return GPacket::Fail;
	}

	packet->ts_.tv_sec = read.pktHdr_.tv_sec_;
	packet->ts_.tv_usec = read.pktHdr_.tv_usec_;
	packet->buf_.data_ = read.data_;
	packet->buf_.size_ = read.pktHdr_.len_;
	id_ = read.id_;
	if (autoParse_) packet->parse();

	return GPacket::Ok;
}

GPacket::Result GNetFilter::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::NotSupported, "not supported");
	return GPacket::Fail;
}

GPacket::Result GNetFilter::write(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NotSupported, "not supported");
	return GPacket::Fail;
}

GPacket::Result GNetFilter::relay(GPacket* packet) {
	GDemon::NfVerdict verdict;
	verdict.id_ = id_;
	verdict.acceptVerdict_ = acceptVerdict_;
	verdict.mark_ = mark_;
	if (packet->ctrl_.changed_) {
		verdict.size_ = packet->buf_.size_;
		verdict.data_ = packet->buf_.data_;
	} else {
		verdict.size_ = 0;
		verdict.data_ = nullptr;
	}
	bool res = demonClient_->nfVerdict(verdict);
	return res ? GPacket::Ok : GPacket::Fail;
}

GPacket::Result GNetFilter::drop(GPacket* packet) {
	(void)packet;
	GDemon::NfVerdict verdict;
	verdict.id_ = id_;
	verdict.acceptVerdict_ = 0; // NF_DROP
	verdict.mark_ = mark_;
	verdict.size_ = 0;
	verdict.data_ = nullptr;
	bool res = demonClient_->nfVerdict(verdict);
	return res ? GPacket::Ok : GPacket::Fail;
}
#else // Q_OS_ANDROID
bool GNetFilter::doOpen() {
	if (!enabled_) return true;

	if (!command_.open()) {
		err = command_.err;
		return false;
	}

	// opening library handle
	h_ = nfq_open();
	if (!h_) {
		SET_ERR(GErr::ReturnNull, "nfq_open return null");
		return false;
	}

	// unbinding existing nf_queue handler for AF_INET (if any)
	if (nfq_unbind_pf(h_, AF_INET) < 0) {
		SET_ERR(GErr::Fail, "error during nfq_unbind_pf()");
		return false;
	}

	// binding nfnetlink_queue as nf_queue handler for AF_INET
	if (nfq_bind_pf(h_, AF_INET) < 0) {
		SET_ERR(GErr::Fail, "error during nfq_bind_pf()");
		return false;
	}

	// binding this socket to queue
	qh_ = nfq_create_queue(h_, queueNum_, cb_, this);
	if (!qh_) {
		SET_ERR(GErr::Fail, "error during nfq_create_queue()");
		return false;
	}

	// setting copy_packet mode
	if (nfq_set_mode(qh_, NFQNL_COPY_PACKET, 0xffff) < 0) {
		SET_ERR(GErr::Fail, "can't set packet_copy mode");
		return false;
	}

	fd_ = nfq_fd(h_);
	if (nonBlock_) {
		int flags = fcntl(fd_, F_GETFL);
		int res = fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
		if (res == -1) {
			SET_ERR(GErr::Fail, QString("fcntl return -1 %1 %2").arg(errno).arg(strerror(errno)));
			return false;
		}
	}

	Q_ASSERT(recvBuf_ == nullptr);
	recvBuf_ = new gbyte[bufSize_];

	id_ = 0;
	ipPacket_ = nullptr;

	return GCapture::doOpen();
}

bool GNetFilter::doClose() {
	if (!enabled_) return true;

	if (fd_ != 0) {
		::shutdown(fd_, 2/* SHUT_RDWR SD_BOTH*/);
		::close(fd_);
		fd_ = 0;
	}

	GCapture::doClose();

	if (qh_ != nullptr) {
		nfq_destroy_queue(qh_);
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
		nfq_close(h_);
		h_ = nullptr;
	}

	if (recvBuf_ != nullptr) {
		delete[] recvBuf_;
		recvBuf_ = nullptr;
	}

	command_.close();

	return true;
}

#ifdef _DEBUG
static int count = 0; // gilgil temp 2021.05.21
#endif // _DEBUG

GPacket::Result GNetFilter::read(GPacket* packet) {
#ifdef _DEBUG
	if (id_ != 0 || ipPacket_ != nullptr) {
		qWarning() << "relay() or drop() must be called after read";
	}
#endif // _DEBUG

	ipPacket_ = PIpPacket(packet);
	int res;
	while (true) {
		if (!active()) return GPacket::Fail;
		// qDebug() << "bef call recv"; // gilgil temp 2021.05.22
		res = int(::recv(fd_, recvBuf_, GPacket::MaxBufSize, 0));
		// qDebug() << "aft call recv" << res; // gilgil temp 2021.05.22
		if (res >= 0) break;
		if (res < 0) {
			if(errno == EWOULDBLOCK) {
				if (waitTimeout_ != 0)
					QThread::msleep(waitTimeout_);
				continue;
			}
			if (errno == ENOBUFS) {
				qWarning() << "losing packets!";
				return GPacket::None;
			}
			qWarning() << QString("recv return %1 errno=%2").arg(res, errno);
			return GPacket::Fail;
		}
	}
	#ifdef _DEBUG
	if (++count != 1)
		qWarning() << "count is" << count;
	#endif // _DEBUG
	nfq_handle_packet(h_, pchar(recvBuf_), res);
	return GPacket::Ok;
}

GPacket::Result GNetFilter::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::NotSupported, "not supported");
	return GPacket::Fail;
}

GPacket::Result GNetFilter::write(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NotSupported, "not supported");
	return GPacket::Fail;
}

GPacket::Result GNetFilter::relay(GPacket* packet) {
	int res;
	if (packet->ctrl_.changed_)
		res = nfq_set_verdict2(qh_, id_, acceptVerdict_, mark_, packet->buf_.size_, packet->buf_.data_);
	else
		res = nfq_set_verdict2(qh_, id_, acceptVerdict_, mark_, 0, nullptr);
	id_ = 0;
	ipPacket_ = nullptr;
	if (res > 0)
		return GPacket::Ok;
	else {
		qWarning() << "nfq_set_verdict2 return " << res;
		return GPacket::Fail;
	}
}

GPacket::Result GNetFilter::drop(GPacket* packet) {
	(void)packet;
	int res = nfq_set_verdict2(qh_, id_, NF_DROP, mark_, 0, nullptr);
	id_ = 0;
	ipPacket_ = nullptr;
	if (res > 0)
		return GPacket::Ok;
	else {
		qWarning() << "nfq_set_verdict2 return " << res;
		return GPacket::Fail;
	}
}

int GNetFilter::_callback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfad, void* data) {
	(void)qh;
	(void)nfmsg;
#ifdef _DEBUG
	if (--count != 0)
		qWarning() << "count is" << count;
#endif // _DEBUG

	GNetFilter* netFilter = static_cast<GNetFilter*>(data);
	Q_ASSERT(netFilter->qh_ == qh);
	GIpPacket* ipPacket = netFilter->ipPacket_;

	ipPacket->clear();
	gettimeofday(&ipPacket->ts_, nullptr);
	int res = nfq_get_payload(nfad, &ipPacket->buf_.data_);
	if (res == -1) {
		qCritical() << "nfq_get_payload return -1";
		return -1;
	}

	if (res > netFilter->bufSize_) {
		qWarning() << QString("res(%1) > bufSize_(%2)").arg(res).arg(netFilter->bufSize_);
		return -1; // gilgil temp 2021.06.17
	}
	ipPacket->buf_.size_ = size_t(res);
	if (netFilter->autoParse_) ipPacket->parse();

	netFilter->id_ = 0;
	struct nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfad);
	if (ph != nullptr)
		netFilter->id_ = ntohl(ph->packet_id);

	return 0;
}
#endif // Q_OS_ANDROID
