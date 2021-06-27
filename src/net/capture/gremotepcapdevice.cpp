#include "gremotepcapdevice.h"

// ----------------------------------------------------------------------------
// GRemotePcapDevice
// ----------------------------------------------------------------------------
GRemotePcapDevice::GRemotePcapDevice(QObject* parent) : GVirtualPcapDevice(parent) {
#ifdef Q_OS_ANDROID
	GRtmEntry* entry = GRemoteNetInfo::instance(ip_, port_).rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GInterface* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
#endif // Q_OS_ANDROID
}

GRemotePcapDevice::~GRemotePcapDevice() {
	close();
}

bool GRemotePcapDevice::doOpen() {
	if (!enabled_) return true;

	if (intfName_ == "") {
		SET_ERR(GErr::INTERFACE_NAME_NOT_SPECIFIED, "intfName is not specified");
		return false;
	}

	demonClient_ = new GDemonClient(std::string(qPrintable(ip_)), port_);
	GDemon::PcapOpenRes res = demonClient_->pcapOpen(qPrintable(objectName()), std::string(qPrintable(filter_)), std::string(qPrintable(intfName_)), snapLen_, flags_, readTimeout_, waitTimeout_, true);
	if (!res.result_) {
		SET_ERR(GErr::FAIL, demonClient_->error_.data());
		delete demonClient_; demonClient_ = nullptr;
		return false;
	}

	intf_ = GRemoteNetInfo::instance(ip_, port_).interfaceList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::VALUE_IS_NULL, msg);
		return false;
	}

	dlt_ = GPacket::intToDlt(res.dataLink_);

	return GCapture::doOpen();
}

bool GRemotePcapDevice::doClose() {
	if (!enabled_) return true;

	intf_ = nullptr;

	if (demonClient_ != nullptr)
		demonClient_->pcapClose();

	GCapture::doClose();

	if (demonClient_ != nullptr) {
		delete demonClient_;
		demonClient_ = nullptr;
	}

	return true;
}

GPacket::Result GRemotePcapDevice::read(GPacket* packet) {
	packet->clear();

	GDemon::PcapRead read = demonClient_->pcapRead();
	if (read.data_ == nullptr) {
		SET_ERR(GErr::READ_FAILED, "read fail");
		return GPacket::Fail;
	}

	packet->ts_.tv_sec = read.pktHdr_.tv_sec;
	packet->ts_.tv_usec = read.pktHdr_.tv_usec;
	packet->buf_.data_ = read.data_;
	packet->buf_.size_ = read.pktHdr_.caplen;
	if (autoParse_) packet->parse();

	return GPacket::Ok;
}

GPacket::Result GRemotePcapDevice::write(GBuf buf) {
	GDemon::PcapWrite write;
	write.size_ = buf.size_;
	write.data_ = buf.data_;
	demonClient_->pcapWrite(write);
	return GPacket::Ok;
}

GPacket::Result GRemotePcapDevice::write(GPacket* packet) {
	return write(packet->buf_);
}

GPacket::Result GRemotePcapDevice::relay(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GRemotePcapDevice::propCreateItem(GPropItemParam* param) {
#ifdef Q_OS_ANDROID
	if (QString(param->mpro_.name()) == "intfName") {
		GPropItemInterface* res = new GPropItemInterface(param);
		res->comboBox_->setEditable(true);
		return res;
	}
#endif // Q_OS_ANDROID
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
