#include "gmonitordevicewrite.h"

GMonitorDeviceWrite::GMonitorDeviceWrite(QObject* parent) : GPcapDeviceWrite(parent) {
#ifdef Q_OS_ANDROID
	port_ = GDemon::NexmonPort;
#endif // Q_OS_ANDROID
	mtu_ = 0; // ignored
}

GMonitorDeviceWrite::~GMonitorDeviceWrite() {
	close();
}

bool GMonitorDeviceWrite::doOpen() {
	if (!enabled_) return true;

	if (!GPcapDeviceWrite::doOpen()) return false;

	GPacket::Dlt _dlt = dlt();
	if (_dlt != GPacket::Dot11) {
		QString msg = QString("Data link type(%1 - %2) must be GPacket::Dot11").arg(intfName_, GPacket::dltToString(_dlt));
		SET_ERR(GErr::Fail, msg);
		return false;
	}

	return true;
}

bool GMonitorDeviceWrite::doClose() {
	if (!enabled_) return true;

	return GPcapDeviceWrite::doClose();
}

GPacket::Result GMonitorDeviceWrite::writeBuf(GBuf buf) {
	return GPcapDeviceWrite::writeBuf(buf);
}

GPacket::Result GMonitorDeviceWrite::write(GPacket* packet) {
	if (!enabled_) return GPacket::Ok;

	GRadioHdr* radioHdr = packet->radioHdr_;
	GDot11* dot11 = packet->dot11_;
	if (radioHdr == nullptr || dot11 == nullptr) {
		GPacket::Result res = GPcapDeviceWrite::writeBuf(packet->buf_);
		if (res == GPacket::Ok)
			emit written(packet);
		return res;
	}

	GRadioHdr* sendRadioHdr = PRadioHdr(sendBuffer_);
	uint32_t sendRadioHdrSize;

	if (clearRadioHdr_) {
		sendRadioHdr->init();
		sendRadioHdrSize = sizeof(GRadioHdr);
	} else {
		memcpy(sendRadioHdr, radioHdr, radioHdr->len_);
		sendRadioHdrSize = radioHdr->len_;
	}

	gbyte* sendDot11 = sendBuffer_ + sendRadioHdrSize;
	uint32_t sendDot11Size = packet->buf_.size_ - radioHdr->len_;

	if (checkFcsSize_) {
		size_t fcsSize = radioHdr->getFcsSize();
		if (fcsSize != 0) {
			if (clearRadioHdr_)
				sendDot11Size -= fcsSize;
			else
				sendDot11Size += fcsSize;
		}
	}
	memcpy(sendDot11, dot11, sendDot11Size);

	GBuf sendBuf(sendBuffer_, sendRadioHdrSize + sendDot11Size);
	GPacket::Result res = GPcapDeviceWrite::writeBuf(sendBuf);
	if (res == GPacket::Ok)
		emit written(packet);
	return res;
}
