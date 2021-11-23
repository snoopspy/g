#include "grawipsocketwrite.h"

// ----------------------------------------------------------------------------
// GRawIpSocketWrite
// ----------------------------------------------------------------------------
bool GRawIpSocketWrite::doOpen() {
	sd_ = ::socket (PF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sd_ == -1) {
		SET_ERR(GErr::FAIL, "socket return -1");
		sd_ = 0;
		return false;
	}

	int one = 1;
	int res = ::setsockopt(sd_, IPPROTO_IP, IP_HDRINCL, pchar(&one), sizeof(one));
	if (res < 0) {
		QString msg = QString("setsockopt return %1").arg(res);
		SET_ERR(GErr::FAIL, msg);
		sd_ = 0;
		return false;
	}

	return true;
}

bool GRawIpSocketWrite::doClose() {
	if (sd_ != 0) {
		::close(sd_);
		sd_ = 0;
	}
	return true;
}

GPacket::Result GRawIpSocketWrite::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GRawIpSocketWrite::write(GPacket* packet) {
	if (packet->ipHdr_ == nullptr) {
		qWarning() << "packet->ipHdr_ is nullptr";
		return GPacket::Fail;
	}

	sockaddr sa;
	sa.sa_family = AF_INET;

	int res = ::sendto(sd_, pchar(packet->ipHdr_), packet->ipHdr_->len(), 0, &sa, sizeof(sa));
	if (res < 0) {
		QString msg = QString("sendto return %1(%2)").arg(res).arg(strerror(errno));
		SET_ERR(GErr::FAIL, msg);
		return GPacket::Fail;
	}
	emit written(packet);
	return GPacket::Ok;
}
