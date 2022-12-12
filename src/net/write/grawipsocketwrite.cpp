#include "grawipsocketwrite.h"

// ----------------------------------------------------------------------------
// GRawIpSocketWrite
// ----------------------------------------------------------------------------
GRawIpSocketWrite::GRawIpSocketWrite(QObject* parent) : GWrite(parent) {
	dlt_ = GPacket::Ip;
}

GRawIpSocketWrite::~GRawIpSocketWrite() {
	close();
}

#ifdef Q_OS_ANDROID
bool GRawIpSocketWrite::doOpen() {
	demonClient_ = new GDemonClient("127.0.0.1", GDemon::DefaultPort);
	GDemon::RiOpenRes res = demonClient_->riOpen();
	if (!res.result_) {
		SET_ERR(GErr::Fail, demonClient_->error_.data());
		delete demonClient_; demonClient_ = nullptr;
		return false;
	}
	return true;
}

bool GRawIpSocketWrite::doClose() {
	if (demonClient_ != nullptr) {
		demonClient_->riClose();
		delete demonClient_;
		demonClient_ = nullptr;
	}
	return true;
}

GPacket::Result GRawIpSocketWrite::write(GBuf buf) {
	GDemon::RiWrite write;
	write.size_ = buf.size_;
	write.data_ = buf.data_;
	bool res = demonClient_->riWrite(write);
	return res ? GPacket::Ok : GPacket::Fail;
}

#else
bool GRawIpSocketWrite::doOpen() {
	sd_ = ::socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sd_ == -1) {
		QString msg = QString("socket return -1 %1").arg(strerror(errno));
		SET_ERR(GErr::Fail, msg);
		return false;
	}

	int one = 1;
	int res = ::setsockopt(sd_, IPPROTO_IP, IP_HDRINCL, pchar(&one), sizeof(one));
	if (res < 0) {
		QString msg = QString("setsockopt(IP_HDRINCL) return %1 %2").arg(res).arg(strerror(errno));
		SET_ERR(GErr::Fail, msg);
		return false;
	}

	if (intfName_ != "") {
		res = ::setsockopt(sd_, SOL_SOCKET, SO_BINDTODEVICE, qPrintable(intfName_), intfName_.size());
		if (res < 0) {
			QString msg = QString("setsockopt(SO_BINDTODEVICE) return %1 %2").arg(res).arg(strerror(errno));
			SET_ERR(GErr::Fail, msg);
			return false;
		}
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

	int res = ::sendto(sd_, pchar(ipHdr), ipHdr->len(), 0, (sockaddr*)&sin, sizeof(sin));
	if (res < 0) {
		QString msg = QString("sendto return %1(%2) buf len=%3").arg(res).arg(strerror(errno)).arg(ipHdr->len());
		SET_ERR(GErr::Fail, msg);
		return GPacket::Fail;
	}
	return GPacket::Ok;
}
#endif

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

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GRawIpSocketWrite::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		return new GPropItemInterface(param);
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
