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

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = packet->ipHdr_->dip_; // network byte order

	if (packet->tcpHdr_ != nullptr)
		sin.sin_port = packet->tcpHdr_->dport_; // network byte order
	else if (packet->udpHdr_ != nullptr)
		sin.sin_port = packet->udpHdr_->dport_; // network byte order
	else
		sin.sin_port = 0; // network byte order

	int res = ::sendto(sd_, pchar(packet->ipHdr_), packet->ipHdr_->len(), 0, (sockaddr*)&sin, sizeof(sin));
	if (res < 0) {
		QString msg = QString("sendto return %1").arg(res);
		SET_ERR(GErr::FAIL, msg);
		return GPacket::Fail;
	}
	emit written(packet);
	return GPacket::Ok;
}
