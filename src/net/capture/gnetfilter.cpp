#include "gnetfilter.h"

#ifdef Q_OS_LINUX

#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

// ----------------------------------------------------------------------------
// GNetFilter
// ----------------------------------------------------------------------------
GNetFilter::GNetFilter(QObject* parent) : GCapture(parent) {
	command_.openCommands_.clear();
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{
#ifdef _DEBUG
		"su -c \"iptables -F\"",
#endif // _DEBUG
		"su -c \"iptables -A OUTPUT -d 127.0.0.1 -j ACCEPT\"",
		"su -c \"iptables -A INPUT -d 127.0.0.1 -j ACCEPT\"",
		"su -c \"iptables -A OUTPUT -j NFQUEUE\"",
		"su -c \"iptables -A INPUT -j NFQUEUE\""}));

	command_.closeCommands_.clear();
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{
		"su -c \"iptables -D OUTPUT -d 127.0.0.1 -j ACCEPT\"",
		"su -c \"iptables -D INPUT -d 127.0.0.1 -j ACCEPT\"",
		"su -c \"iptables -D OUTPUT -j NFQUEUE\"",
		"su -c \"iptables -D INPUT -j NFQUEUE\""}));

	cb_ = &_callback;
}

GNetFilter::~GNetFilter() {
	close();
}

bool GNetFilter::doOpen() {
	if (!enabled_) return true;

	if (!command_.open()) {
		err = command_.err;
		return false;
	}

	// opening library handle
	h_ = nfq_open();
	if (!h_) {
		SET_ERR(GErr::RETURN_NULL, "nfq_open return null");
		return false;
	}

	// unbinding existing nf_queue handler for AF_INET (if any)
	if (nfq_unbind_pf(h_, AF_INET) < 0) {
		SET_ERR(GErr::FAIL, "error during nfq_unbind_pf()");
		return false;
	}

	// binding nfnetlink_queue as nf_queue handler for AF_INET
	if (nfq_bind_pf(h_, AF_INET) < 0) {
		SET_ERR(GErr::FAIL, "error during nfq_bind_pf()");
		return false;
	}

	// binding this socket to queue
	qh_ = nfq_create_queue(h_, uint16_t(queueNum_), cb_, this);
	if (!qh_) {
		SET_ERR(GErr::FAIL, "error during nfq_create_queue()");
		return false;
	}

	// setting copy_packet mode
	if (nfq_set_mode(qh_, NFQNL_COPY_PACKET, 0xffff) < 0) {
		SET_ERR(GErr::FAIL, "can't set packet_copy mode");
		return false;
	}

	fd_ = nfq_fd(h_);
	qDebug() << QString("fd=%1").arg(fd_); // gilgil temp 2016.09.25

	recvBuf_ = new char[GPacket::MaxBufSize];

	id_ = 0;
	ipPacket_ = nullptr;

	return GCapture::doOpen();
}

bool GNetFilter::doClose() {
	if (!enabled_) return true;

	if (fd_ != 0) {
		qDebug() << "bef call ::shutdown"; // gilgil temp 2016.09.25
		::shutdown(fd_, SHUT_RDWR);
		qDebug() << "aft call ::shutdown"; // gilgil temp 2016.09.25

		qDebug() << "bef call ::close"; // gilgil temp 2016.09.25
		::close(fd_);
		qDebug() << "aft call ::close"; // gilgil temp 2016.09.25
		fd_ = 0;
	}

	GCapture::doClose();

	if (qh_ != nullptr) {
		qDebug() << "bef call nfq_destroy_queue"; // gilgil temp 2016.09.25
		nfq_destroy_queue(qh_);
		qDebug() << "aft call nfq_destroy_queue"; // gilgil temp 2016.09.25
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
		qDebug() << "bef call nfq_close"; // gilgil temp 2016.09.25
		nfq_close(h_);
		qDebug() << "aft call nfq_close"; // gilgil temp 2016.09.25
		h_ = nullptr;
	}

	if (recvBuf_ != nullptr) {
		delete recvBuf_;
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
		qWarning() << "relay() or drop() called after read?";
	}
#endif // _DEBUG

	ipPacket_ = PIpPacket(packet);
	// qDebug() << "bef call recv"; // gilgil temp 2021.05.22
	int res = int(::recv(fd_, recvBuf_, GPacket::MaxBufSize, 0));
	// qDebug() << "aft call recv" << res; // gilgil temp 2021.05.22
	if (res >= 0) {
		#ifdef _DEBUG
		if (++count != 1)
			qWarning() << "count is" << count;
		#endif // _DEBUG
		nfq_handle_packet(h_, recvBuf_, res);
	} else {
		if (errno == ENOBUFS) {
			qWarning() << "losing packets!";
			return GPacket::None;
		}
		qWarning() << "recv failed";
		return GPacket::Fail;
	}
	if (ipPacket_ != nullptr)
		return GPacket::Ok;
	else
		return GPacket::None;
}

GPacket::Result GNetFilter::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GNetFilter::write(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GNetFilter::relay(GPacket* packet) {
	int res;
	if (packet->ctrl.changed_)
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
	ipPacket->buf_.size_ = size_t(res);
	if (netFilter->autoParse_) ipPacket->parse();

	netFilter->id_ = 0;
	struct nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfad);
	if (ph != nullptr)
		netFilter->id_ = ntohl(ph->packet_id);

	return 0;
}

#endif
