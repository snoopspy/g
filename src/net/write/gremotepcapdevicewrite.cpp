#include "gremotepcapdevicewrite.h"

// ----------------------------------------------------------------------------
// GRemotePcapDeviceWrite
// ----------------------------------------------------------------------------
GRemotePcapDeviceWrite::GRemotePcapDeviceWrite(QObject* parent) : GPcapDeviceWrite(parent) {
	GRtmEntry* entry = GRemoteNetInfo::instance(ip_, port_).rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GInterface* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
}

GRemotePcapDeviceWrite::~GRemotePcapDeviceWrite() {
	close();
}

bool GRemotePcapDeviceWrite::doOpen() {
	if (intfName_ == "") {
		SET_ERR(GErr::INTERFACE_NAME_NOT_SPECIFIED, "intfName is not specified");
		return false;
	}

	demonClient_ = new GDemonClient(std::string(qPrintable(ip_)), port_);
	GDemon::PcapOpenRes res = demonClient_->pcapOpen("", std::string(qPrintable(intfName_)), 0, 0, 0, 0, false);
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
	return true;
}

bool GRemotePcapDeviceWrite::doClose() {
	if (demonClient_ != nullptr) {
		demonClient_->pcapClose();
		delete demonClient_;
		demonClient_ = nullptr;
	}

	return true;
}

GPacket::Result GRemotePcapDeviceWrite::write(GBuf buf) {
	GDemon::PcapWrite write;
	write.size_ = buf.size_;
	write.data_ = buf.data_;
	demonClient_->pcapWrite(write);
	return GPacket::Ok;
}

GPacket::Result GRemotePcapDeviceWrite::write(GPacket* packet) {
	GPacket::Result res = write(packet->buf_);
	if (res == GPacket::Ok)
		emit written(packet);
	return res;
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GRemotePcapDeviceWrite::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		GPropItemInterface* res = new GPropItemInterface(param);
		res->comboBox_->setEditable(true);
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
