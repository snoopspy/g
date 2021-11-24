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
	GIpHdr* ipHdr = PIpHdr(buf.data_);

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ipHdr->dip_; // network byte order

	errno = 0; // gilgil temp 2021.11.24
	int res = ::sendto(sd_, pbyte(ipHdr), ipHdr->len(), 0, (sockaddr*)&sin, sizeof(sin));
	if (res < 0) {
		QString msg = QString("sendto return %1(%2) buf len=%3").arg(res).arg(strerror(errno)).arg(ipHdr->len());
		SET_ERR(GErr::FAIL, msg);
		return GPacket::Fail;
	}
	return GPacket::Ok;
}

GPacket::Result GRawIpSocketWrite::write(GPacket* packet) {
	GPacket::Result res;
	if (mtu_ != 0 && packet->ipHdr_ != nullptr && packet->ipHdr_->len() > uint16_t(mtu_) && packet->tcpHdr_ != nullptr) {
		GBuf backupBuf = packet->buf_;
		GIpHdr* ipHdr = packet->ipHdr_ ;
		packet->buf_.data_ = pbyte(ipHdr);
		packet->buf_.size_ = ipHdr->len();
		res = writeMtuSplit(packet, mtu_, GPacket::Ip);
		packet->buf_ = backupBuf;
	} else
		res = write(GBuf(pbyte(packet->ipHdr_), packet->ipHdr_->len()));

	if (res == GPacket::Ok)
		emit written(packet);
	return res;
}
