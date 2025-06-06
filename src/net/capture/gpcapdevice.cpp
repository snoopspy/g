#include "gpcapdevice.h"

// ----------------------------------------------------------------------------
// GPcapDevice
// ----------------------------------------------------------------------------
GPcapDevice::GPcapDevice(QObject* parent) : GPcapCapture(parent) {
#ifndef Q_OS_ANDROID
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GIntf* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
#else
	intfName_ = "wlan0";
#endif // Q_OS_ANDROID
}

GPcapDevice::~GPcapDevice() {
	close();
}

bool GPcapDevice::doOpen() {
	if (!enabled_) return true;

	if (intfName_ == "") {
		SET_ERR(GErr::InterfaceNameNotSpecified, "intfName is not specified");
		return false;
	}

#ifdef Q_OS_ANDROID
	demonClient_ = new GDemonClient("127.0.0.3", port_);
	GDemon::PcapOpenRes res = demonClient_->pcapOpen(std::string(qPrintable(objectName())), std::string(qPrintable(filter_)), std::string(qPrintable(intfName_)), snapLen_, flags_, readTimeout_, waitTimeout_, nonBlock_, true);
	if (!res.result_) {
		SET_ERR(GErr::Fail, demonClient_->error_.data());
		delete demonClient_; demonClient_ = nullptr;
		return false;
	}

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::ValueIsNull, msg);
		return false;
	}

	dlt_ = GPacket::intToDlt(res.dataLink_);
#else
	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_live(qPrintable(intfName_), snapLen_, flags_, readTimeout_, errBuf);
	if (pcap_ == nullptr) {
		SET_ERR(GErr::ReturnNull, errBuf);
		return false;
	}

	if (nonBlock_) {
		// ----- gilgil temp 2023.11.18 -----
		int nonblock = pcap_getnonblock(pcap_, errBuf);
		if (nonblock == -1)
			qDebug() << QString("pcap_getnonblock return -1 %s").arg(errBuf);
		else
			qDebug("pcap_getnonblock return %d", nonblock);
		// ----------------------------------

		int res = pcap_setnonblock(pcap_, 1, errBuf);
		if (res == PCAP_ERROR) {
			SET_ERR(GErr::Fail, errBuf);
			return false;
		}

		// ----- gilgil temp 2023.11.18 -----
		nonblock = pcap_getnonblock(pcap_, errBuf);
		if (nonblock == -1)
			qDebug() << QString("pcap_getnonblock return -1 %s").arg(errBuf);
		else
			qDebug("pcap_getnonblock return %d", nonblock);
		// ----------------------------------
	}
#endif

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::ValueIsNull, msg);
		return false;
	}

	//pcap_setnonblock(pcap_, 1, errBuf);

#ifdef Q_OS_ANDROID
	return GCapture::doOpen();
#else
	return GPcapCapture::doOpen();
#endif
}

bool GPcapDevice::doClose() {
	if (!enabled_) return true;

	intf_ = nullptr;

#ifdef Q_OS_ANDROID
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

#ifdef Q_OS_ANDROID
GPacket::Result GPcapDevice::read(GPacket* packet) {
	packet->clear();

	GDemon::PcapRead read = demonClient_->pcapRead();
	if (read.data_ == nullptr) {
		SET_ERR(GErr::ReadFailed, "read fail");
		return GPacket::Fail;
	}

	packet->ts_.tv_sec = read.pktHdr_.tv_sec_;
	packet->ts_.tv_usec = read.pktHdr_.tv_usec_;
	packet->buf_.data_ = read.data_;
	packet->buf_.size_ = read.pktHdr_.caplen_;
	if (autoParse_) packet->parse();

	return GPacket::Ok;
}

GPacket::Result GPcapDevice::writeBuf(GBuf buf) {
	GDemon::PcapWrite write;
	write.size_ = buf.size_;
	write.data_ = buf.data_;
	demonClient_->pcapWrite(write);
	return GPacket::Ok;
}

GPacket::Result GPcapDevice::write(GPacket* packet) {
	return writeBuf(packet->buf_);
}

GPacket::Result GPcapDevice::relay(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NotSupported, "not supported");
	return GPacket::Fail;
}
#else
GPacket::Result GPcapDevice::read(GPacket* packet) {
	GPacket::Result res = GPcapCapture::read(packet);
	if (res == GPacket::None && waitTimeout_ != 0)
		QThread::msleep(waitTimeout_);
	return res;
}
#endif

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GPcapDevice::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		return new GPropItemInterface(param);
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
