#include "gmonitordevice.h"
#include "gsyncpcapdevice.h"
#include "net/iw/giw.h"
#include "net/packet/gdot11packet.h"

GMonitorDevice::GMonitorDevice(QObject* parent) : GPcapDevice(parent) {
#ifdef Q_OS_ANDROID
    port_ = GDemon::NexmonPort;
#endif // Q_OS_ANDROID
}

GMonitorDevice::~GMonitorDevice() {
	close();
}

bool GMonitorDevice::doOpen() {
	if (!enabled_) return true;

	bool res = false;

	if (checkRadioInfo_) {
		if (!getRadioInfoFromFile()) {
			if (!getRadioInfoFromDevice()) {
				SET_ERR(GErr::Fail, "cat not get radio info");
				return false;
			}
		}

		QString backupFilter = filter_;
		uint16_t radioLen = htons(radioInfo_.len_);
		QString lengthFilter = QString("radio[2:2]==%1").arg(radioLen);
		if (filter_ == "")
			filter_ = lengthFilter;
		else
			filter_ = QString("(%1) and (%2)").arg(filter_, lengthFilter);

		res = GPcapDevice::doOpen();
		filter_ = backupFilter;
	} else {
		res = GPcapDevice::doOpen();
	}

	if (res) {
		GPacket::Dlt _dlt = dlt();
		if (_dlt != GPacket::Dot11) {
			QString msg = QString("Data link type(%1 - %2) must be GPacket::Dot11").arg(intfName_, GPacket::dltToString(_dlt));
			SET_ERR(GErr::Fail, msg);
			return false;
		}
	}
	return res;
}

bool GMonitorDevice::doClose() {
	if (!enabled_) return true;

	return GPcapDevice::doClose();
}

bool GMonitorDevice::getRadioInfoFromFile() {
	GIntf* intf = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf == nullptr) {
		QString msg = QString("intf(%1) is null").arg(intfName_);
		SET_ERR(GErr::Fail, msg);
		return -1;
	}
	GMac mac = intf->mac();
	FILE* fp = fopen("radioinfo.txt", "rt");
	if (fp != nullptr) {
		while (true) {
			char macBuf[256];
			int len;
			int fcsSize;
			int res = fscanf(fp, "%s %d %d", macBuf, &len, &fcsSize);
			if (res != 3) {
				qWarning() << "fscanf return" << res;
				return false;
			}
			if (mac == GMac(macBuf)) {
				qDebug() << QString("file radioInfo len=%1 fcsSize=%2").arg(len).arg(fcsSize);
				radioInfo_.len_ = len;
				radioInfo_.fcsSize_  = fcsSize;
				return true;
			}
		}
		fclose(fp);
	}
	return false;
}

bool GMonitorDevice::getRadioInfoFromDevice() {
	GSyncPcapDevice device;
#ifdef Q_OS_ANDROID
    device.port_ = GDemon::NexmonPort;
#endif // Q_OS_ANDROID
	device.intfName_ = intfName_;
	if (!device.open()) {
		err = device.err;
		return false;
	}

	GPacket::Dlt _dlt = device.dlt();
	if (_dlt != GPacket::Dot11) {
		QString msg = QString("Data link type(%1 - %2) must be GPacket::Dot11").arg(intfName_).arg(GPacket::dltToString(_dlt));
		SET_ERR(GErr::Fail, msg);
		device.close();
		return false;
	}

	GIw iw;
	if (!iw.open(intfName_)) {
		SET_ERR(GErr::Fail, iw.error_);
		return false;
	}
	if (!iw.setChannel(1)) {
		SET_ERR(GErr::Fail, iw.error_);
		return false;
	}
	iw.close();

	int len = -1;
	while (true) {
		GDot11Packet packet;
		GPacket::Result res = device.read(&packet);
		switch (res) {
			case GPacket::Eof:
				SET_ERR(GErr::Fail, "device.read return Eof");
				device.close();
				return false;
			case GPacket::Fail:
				SET_ERR(GErr::Fail, "device.read return Fail");
				device.close();
				return false;
			case GPacket::None:
				continue;
			case GPacket::Ok:
				break;
		}
		GRadioHdr* radioHdr = packet.radioHdr_;
		if (radioHdr == nullptr) {
			SET_ERR(GErr::ObjectIsNull, "radioHdr is null");
			return false;
		}
		len = radioHdr->len_;

		uint32_t fcsSize = radioHdr->getFcsSize();

		qDebug() << QString("device radioInfo len=%1 fcsSize=%2").arg(len).arg(fcsSize);
		radioInfo_.len_ = len;
		radioInfo_.fcsSize_  = fcsSize;

		GIntf* intf = GNetInfo::instance().intfList().findByName(intfName_);
		if (intf == nullptr) {
			QString msg = QString("intf(%1) is null").arg(intfName_);
			SET_ERR(GErr::Fail, msg);
			device.close();
			return false;
		}
		GMac mac = intf->mac();
		FILE* fp = fopen("radioinfo.txt", "at");
		if (fp != nullptr) {
			fprintf(fp, "%s %d %d\n", qPrintable(QString(mac)), len, fcsSize);
			fclose(fp);
		}
		break;
	}
	device.close();

	return true;
}
