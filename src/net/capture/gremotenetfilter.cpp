#include "gremotenetfilter.h"

// ----------------------------------------------------------------------------
// GRemoteNetFilter
// ----------------------------------------------------------------------------
GRemoteNetFilter::GRemoteNetFilter(QObject* parent) : GVirtualNetFilter(parent) {
	command_.openCommands_.clear();
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{
#ifdef _DEBUG
		"su -c \"iptables -F\"",
#endif // _DEBUG
		"su -c \"iptables -A OUTPUT -d 127.0.0.1 -j ACCEPT\"",
		"su -c \"iptables -A INPUT -d 127.0.0.1 -j ACCEPT\"",
		"su -c \"iptables -A OUTPUT -j NFQUEUE\"",
		"su -c \"iptables -A INPUT -j NFQUEUE\""
	}));

	command_.closeCommands_.clear();
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{
		"su -c \"iptables -D INPUT -j NFQUEUE\"",
		"su -c \"iptables -D OUTPUT -j NFQUEUE\"",
		"su -c \"iptables -D INPUT -d 127.0.0.1 -j ACCEPT\"",
		"su -c \"iptables -D OUTPUT -d 127.0.0.1 -j ACCEPT\"",
	}));
}

GRemoteNetFilter::~GRemoteNetFilter() {
	close();
}

bool GRemoteNetFilter::doOpen() {
	if (!enabled_) return true;

	if (!command_.open()) {
		err = command_.err;
		return false;
	}

	demonClient_ = new GDemonClient(std::string(qPrintable(ip_)), port_);
	GDemon::NfOpenRes res = demonClient_->nfOpen(qPrintable(objectName()), queueNum_);
	if (!res.result_) {
		SET_ERR(GErr::FAIL, demonClient_->error_.data());
		delete demonClient_; demonClient_ = nullptr;
		return false;
	}

	return GCapture::doOpen();
}

bool GRemoteNetFilter::doClose() {
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

GPacket::Result GRemoteNetFilter::read(GPacket* packet) {
	packet->clear();

	GDemon::NfRead read = demonClient_->nfRead();
	if (read.data_ == nullptr) {
		SET_ERR(GErr::READ_FAILED, "read fail");
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

GPacket::Result GRemoteNetFilter::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GRemoteNetFilter::write(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GRemoteNetFilter::relay(GPacket* packet) {
	GDemon::NfVerdict verdict;
	verdict.id_ = id_;
	verdict.acceptVerdict_ = acceptVerdict_;
	verdict.mark_ = mark_;
	if (packet->ctrl.changed_) {
		verdict.size_ = packet->buf_.size_;
		verdict.data_ = packet->buf_.data_;
	} else {
		verdict.size_ = 0;
		verdict.data_ = nullptr;
	}
	bool res = demonClient_->nfVerdict(verdict);
	return res ? GPacket::Ok : GPacket::Fail;
}

GPacket::Result GRemoteNetFilter::drop(GPacket* packet) {
	(void)packet;
	GDemon::NfVerdict verdict;
	verdict.id_ = id_;
	verdict.acceptVerdict_ = NF_DROP;
	verdict.mark_ = mark_;
	verdict.size_ = 0;
	verdict.data_ = nullptr;
	bool res = demonClient_->nfVerdict(verdict);
	return res ? GPacket::Ok : GPacket::Fail;
}
