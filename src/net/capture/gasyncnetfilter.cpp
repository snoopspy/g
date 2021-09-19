#include "gasyncnetfilter.h"

#if defined(Q_OS_LINUX) && (!defined(Q_OS_ANDROID) || !defined(Q_OS_ANDROID_GILGIL))

// ----------------------------------------------------------------------------
// GAsyncNetFilter
// ----------------------------------------------------------------------------
GAsyncNetFilter::GAsyncNetFilter(QObject* parent) : GNetFilter(parent) {
	cb_ = &_asyncCallback;
}

GAsyncNetFilter::~GAsyncNetFilter() {
	close();
}

bool GAsyncNetFilter::doOpen() {
	return GNetFilter::doOpen();
}

bool GAsyncNetFilter::doClose() {
	return GNetFilter::doClose();
}

GPacket::Result GAsyncNetFilter::read(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GAsyncNetFilter::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GAsyncNetFilter::write(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GAsyncNetFilter::relay(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

void GAsyncNetFilter::run() {
	qDebug() << "beg"; // gilgil temp 2016.09.27
	while (active()) {
		// qDebug() << "bef call recv"; // gilgil temp 2016.09.27
		int res = int(::recv(fd_, recvBuf_, GPacket::MaxBufSize, 0));
		// qDebug() << "aft call recv" << res; // gilgil temp 2016.09.27
		if (res < 0) {
			if (errno == EWOULDBLOCK) {
				if (waitTimeout_ != 0)
					QThread::msleep(waitTimeout_);
				continue;
			}
			if (errno == ENOBUFS) {
				qWarning() << "losing packets!";
				continue;
			}
			qWarning() << QString("recv return %1 errno=%2").arg(res, errno);
			break;
		}
		nfq_handle_packet(h_, pchar(recvBuf_), res);
	}
	emit closed();
	qDebug() << "end"; // gilgil temp 2016.09.27
}

int GAsyncNetFilter::_asyncCallback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfad, void* data) {
	(void)nfmsg;

	GAsyncNetFilter* asyncNetFilter = static_cast<GAsyncNetFilter*>(data);
	GIpPacket* ipPacket = &asyncNetFilter->asyncIpPacket_;

	ipPacket->clear();
	gettimeofday(&ipPacket->ts_, nullptr);
	int res = nfq_get_payload(nfad, &ipPacket->buf_.data_);
	if (res == -1) {
		qCritical() << "nfq_get_payload return -1";
		return -1;
	}
	ipPacket->buf_.size_ = size_t(res);
	if (asyncNetFilter->autoParse_) ipPacket->parse();
	emit asyncNetFilter->captured(ipPacket);

	uint32_t id = 0;
	struct nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfad);
	if (ph != nullptr)
		id = ntohl(ph->packet_id);

	if (ipPacket->ctrl.block_)
		res = nfq_set_verdict2(qh, id, NF_DROP, asyncNetFilter->mark_, 0, nullptr);
	else if (ipPacket->ctrl.changed_)
		res = nfq_set_verdict2(qh, id, asyncNetFilter->acceptVerdict_, asyncNetFilter->mark_, ipPacket->buf_.size_, ipPacket->buf_.data_);
	else
		res = nfq_set_verdict2(qh, id, asyncNetFilter->acceptVerdict_, asyncNetFilter->mark_, 0, nullptr);
	return res;
}

#endif
