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
	sockaddr sa;
	sa.sa_family = AF_INET;
	int res = ::sendto(sd_, buf.data_, buf.size_, 0, &sa, sizeof(sa));
	if (res < 0) {
		QString msg = QString("sendto return %1(%2)").arg(res).arg(strerror(errno));
		SET_ERR(GErr::FAIL, msg);
		return GPacket::Fail;
	}
	return GPacket::Ok;
}

GPacket::Result GRawIpSocketWrite::write(GPacket* packet) {
	Q_ASSERT(packet->ipHdr_ != nullptr);

	GPacket::Result res = write(GBuf(pbyte(packet->ipHdr_), packet->ipHdr_->len()));
	if (res == GPacket::Ok)
		emit written(packet);
	return res;
}
