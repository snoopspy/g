#include "gpcapdevice.h"

// ----------------------------------------------------------------------------
// GPcapDevice
// ----------------------------------------------------------------------------
GPcapDevice::GPcapDevice(QObject* parent) : GPcapCapture(parent) {
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GIntf* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
}

GPcapDevice::~GPcapDevice() {
	close();
}

bool GPcapDevice::doOpen() {
	if (!enabled_) return true;

	if (intfName_ == "") {
		SET_ERR(GErr::INTERFACE_NAME_NOT_SPECIFIED, "intfName is not specified");
		return false;
	}

#if defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID_GILGIL)
	demonClient_ = new GDemonClient("127.0.0.1", GDemon::DefaultPort);
	GDemon::PcapOpenRes res = demonClient_->pcapOpen(qPrintable(objectName()), std::string(qPrintable(filter_)), std::string(qPrintable(intfName_)), snapLen_, flags_, readTimeout_, waitTimeout_, true);
	if (!res.result_) {
		SET_ERR(GErr::FAIL, demonClient_->error_.data());
		delete demonClient_; demonClient_ = nullptr;
		return false;
	}

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::VALUE_IS_NULL, msg);
		return false;
	}

	dlt_ = GPacket::intToDlt(res.dataLink_);
#else
	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_live(qPrintable(intfName_), snapLen_, flags_, readTimeout_, errBuf);
	if (pcap_ == nullptr) {
		SET_ERR(GErr::RETURN_NULL, errBuf);
		return false;
	}
#endif

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::VALUE_IS_NULL, msg);
		return false;
	}

#if defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID_GILGIL)
	return GCapture::doOpen();
#else
	return GPcapCapture::doOpen();
#endif
}

bool GPcapDevice::doClose() {
	if (!enabled_) return true;

	intf_ = nullptr;

#if defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID_GILGIL)
	if (demonClient_ != nullptr)
		demonClient_->pcapClose();

	GCapture::doClose();

	if (demonClient_ != nullptr) {
		delete demonClient_;
		demonClient_ = nullptr;
	}

	return true;
#else
	return GPcapCapture::doClose();
#endif
}

#if defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID_GILGIL)
GPacket::Result GPcapDevice::read(GPacket* packet) {
	packet->clear();

	GDemon::PcapRead read = demonClient_->pcapRead();
	if (read.data_ == nullptr) {
		SET_ERR(GErr::READ_FAILED, "read fail");
		return GPacket::Fail;
	}

	packet->ts_.tv_sec = read.pktHdr_.tv_sec_;
	packet->ts_.tv_usec = read.pktHdr_.tv_usec_;
	packet->buf_.data_ = read.data_;
	packet->buf_.size_ = read.pktHdr_.caplen_;
	if (autoParse_) packet->parse();

	return GPacket::Ok;
}

GPacket::Result GPcapDevice::write(GBuf buf) {
	GDemon::PcapWrite write;
	write.size_ = buf.size_;
	write.data_ = buf.data_;
	demonClient_->pcapWrite(write);
	return GPacket::Ok;
}

GPacket::Result GPcapDevice::write(GPacket* packet) {
	return write(packet->buf_);
}

GPacket::Result GPcapDevice::relay(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}
#else
GPacket::Result GPcapDevice::read(GPacket* packet) {
	GPacket::Result res = GPcapCapture::read(packet);
	if (res == GPacket::None && waitTimeout_ != 0) QThread::msleep(waitTimeout_);
	return res;
}
#endif

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GPcapDevice::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		GPropItemInterface* res = new GPropItemInterface(param);
#if defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID_GILGIL)
		res->comboBox_->setEditable(true);
#endif
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
