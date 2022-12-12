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
	return GPcapDeviceWrite::doClose();
}

GPacket::Result GMonitorDeviceWrite::write(GBuf buf) {
	return GPcapDeviceWrite::write(buf);
}

GPacket::Result GMonitorDeviceWrite::write(GPacket* packet) {
	if (checkFcsSize_) {
		GRadioHdr* radioHdr = packet->radioHdr_;
		if (radioHdr != nullptr) {
			size_t fcsSize = radioHdr->getFcsSize();
			if (fcsSize != 0) {
				GBuf backupBuf = packet->buf_;
				packet->buf_.size_ += fcsSize;
				GPacket::Result res = GPcapDeviceWrite::write(packet->buf_);
				if (res == GPacket::Ok)
					emit written(packet);
				packet->buf_ = backupBuf;
				return res;
			}
		}
	}
	GPacket::Result res = GPcapDeviceWrite::write(packet->buf_);
	if (res == GPacket::Ok)
		emit written(packet);
	return res;
}
