#include "gpcapdevicewrite.h"

// ----------------------------------------------------------------------------
// GPcapDeviceWrite
// ----------------------------------------------------------------------------
GPcapDeviceWrite::GPcapDeviceWrite(QObject* parent) : GPcapWrite(parent) {
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

GPcapDeviceWrite::~GPcapDeviceWrite() {
	close();
}

#ifdef Q_OS_ANDROID
bool GPcapDeviceWrite::doOpen() {
	if (!enabled_) return true;

	if (intfName_ == "") {
		SET_ERR(GErr::InterfaceNameNotSpecified, "intfName is not specified");
		return false;
	}

	demonClient_ = new GDemonClient("127.0.0.5", port_);
	GDemon::PcapOpenRes res = demonClient_->pcapOpen(std::string(qPrintable(objectName())), "", std::string(qPrintable(intfName_)), 0, 0, 0, 0, false, false);
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
	return true;
}

bool GPcapDeviceWrite::doClose() {
	if (!enabled_) return true;

	if (demonClient_ != nullptr) {
		demonClient_->pcapClose();
		delete demonClient_;
		demonClient_ = nullptr;
	}
	return true;
}

GPacket::Result GPcapDeviceWrite::writeBuf(GBuf buf) {
	GDemon::PcapWrite write;
	write.size_ = buf.size_;
	write.data_ = buf.data_;
	bool res = demonClient_->pcapWrite(write);
	return res ? GPacket::Ok : GPacket::Fail;
}

GPacket::Result GPcapDeviceWrite::write(GPacket* packet) {
	if (!enabled_) return GPacket::Ok;

	GPacket::Result res = writeBuf(packet->buf_);
	if (res == GPacket::Ok)
		emit written(packet);
	return res;
}
#else
bool GPcapDeviceWrite::doOpen() {
	if (!enabled_) return true;

	if (intfName_ == "") {
		SET_ERR(GErr::InterfaceNameNotSpecified, "intfName is not specified");
		return false;
	}

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_live(qPrintable(intfName_), 0, 0, 0, errBuf);
	if (pcap_ == nullptr) {
		SET_ERR(GErr::ReturnNull, errBuf);
		return false;
	}

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::ValueIsNull, msg);
		return false;
	}

	int dataLink = pcap_datalink(pcap_);
	dlt_ = GPacket::intToDlt(dataLink);

	return true;
}

bool GPcapDeviceWrite::doClose() {
	if (!enabled_) return true;

	if (pcap_ != nullptr) {
		pcap_close(pcap_);
		pcap_ = nullptr;
	}

	intf_ = nullptr;

	return true;
}

GPacket::Result GPcapDeviceWrite::writeBuf(GBuf buf) {
	int i = pcap_sendpacket(pcap_, buf.data_, int(buf.size_));
	if (i != 0) {
		SET_ERR(GErr::Fail, QString("pcap_sendpacket return %1 %2 size=%3").arg(i).arg(pcap_geterr(pcap_)).arg(buf.size_));
		return GPacket::Fail;
	}
	return GPacket::Ok;
}

GPacket::Result GPcapDeviceWrite::write(GPacket* packet) {
	if (!enabled_) return GPacket::Ok;

	GPacket::Result res;
	if (mtu_ != 0 && packet->ipHdr_ != nullptr && packet->ipHdr_->tlen() > uint16_t(mtu_) && packet->tcpHdr_ != nullptr)
		res = writeMtuSplit(packet, mtu_, GPacket::Eth);
	else
		res = writeBuf(packet->buf_);
	if (res == GPacket::Ok)
		emit written(packet);
	return res;
}
#endif

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GPcapDeviceWrite::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		return new GPropItemInterface(param);
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
