#include "gpcapdevicewrite.h"

// ----------------------------------------------------------------------------
// GPcapDeviceWrite
// ----------------------------------------------------------------------------
GPcapDeviceWrite::GPcapDeviceWrite(QObject* parent) : GVirtualPcapDeviceWrite(parent) {
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GInterface* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
}

GPcapDeviceWrite::~GPcapDeviceWrite() {
	close();
}

bool GPcapDeviceWrite::doOpen() {
	if (intfName_ == "") {
		SET_ERR(GErr::INTERFACE_NAME_NOT_SPECIFIED, "intfName is not specified");
		return false;
	}

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_live(qPrintable(intfName_), 0, 0, 0, errBuf);
	if (pcap_ == nullptr) {
		SET_ERR(GErr::RETURN_NULL, errBuf);
		return false;
	}

	intf_ = GNetInfo::instance().interfaceList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::VALUE_IS_NULL, msg);
		return false;
	}

	int dataLink = pcap_datalink(pcap_);
	dlt_ = GPacket::intToDlt(dataLink);

	return true;
}

bool GPcapDeviceWrite::doClose() {
	if (pcap_ != nullptr) {
		pcap_close(pcap_);
		pcap_ = nullptr;
	}

	intf_ = nullptr;

	return true;
}

GPacket::Result GPcapDeviceWrite::write(GBuf buf) {
	int i = pcap_sendpacket(pcap_, buf.data_, int(buf.size_));
	if (i != 0) {
		SET_ERR(GErr::FAIL, QString("pcap_sendpacket return %1 %2 size=%3").arg(i).arg(pcap_geterr(pcap_)).arg(buf.size_));
		return GPacket::Fail;
	}
	return GPacket::Ok;
}

GPacket::Result GPcapDeviceWrite::write(GPacket* packet) {
	GPacket::Result res;
	if (mtu_ != 0 && packet->buf_.size_ > sizeof(GEthHdr) + mtu_ && dlt_ == GPacket::Eth && packet->tcpHdr_ != nullptr)
		res = writeMtuSplit(packet, mtu_);
	else
		res = write(packet->buf_);
	if (res == GPacket::Ok)
		emit written(packet);
	return res;
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GPcapDeviceWrite::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		GPropItemInterface* res = new GPropItemInterface(param);
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
