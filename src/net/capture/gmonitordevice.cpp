#include "gmonitordevice.h"
#include "gsyncpcapdevice.h"
#include "net/packet/gdot11packet.h"

GMonitorDevice::GMonitorDevice(QObject* parent) : GPcapDevice(parent) {
}

GMonitorDevice::~GMonitorDevice() {
	close();
}

bool GMonitorDevice::doOpen() {
	bool res = false;
	int16_t radioLen = -1;

	if (checkRadioLen_) {
		radioLen = getRadioLenFromFile();
		if (radioLen == -1)
			radioLen = getRadioLenFromDevice();
		if (radioLen == -1) {
			SET_ERR(GErr::Fail, "Count not get radio len");
			return false;
		}

		QString backupFilter = filter_;
		radioLen = htons(radioLen);
		QString lengthFilter = QString("radio[2:2]==%1").arg(radioLen);
		if (filter_ == "")
			filter_ = lengthFilter;
		else
			filter_ = QString("(%1) and (%2)").arg(filter_).arg(lengthFilter);

		res = GPcapDevice::doOpen();
		filter_ = backupFilter;
	} else {
		res = GPcapDevice::doOpen();
	}

	if (res) {
		GPacket::Dlt _dlt = dlt();
		if (_dlt != GPacket::Dot11) {
			QString msg = QString("Data link type(%1) must be GPacket::Dot11").arg(GPacket::dltToString(_dlt));
			SET_ERR(GErr::Fail, msg);
			return false;
		}
	}
	return res;
}

bool GMonitorDevice::doClose() {
	return GPcapDevice::doClose();
}

int16_t GMonitorDevice::getRadioLenFromFile() {
	int radioLen = -1;

	GIntf* intf = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf == nullptr) {
		QString msg = QString("intf(%1) is null").arg(intfName_);
		SET_ERR(GErr::Fail, msg);
		return -1;
	}
	GMac mac = intf->mac();
	FILE* fp = fopen("radiolen.txt", "rt");
	if (fp != nullptr) {
		while (true) {
			char macBuf[256];
			int r;
			int res = fscanf(fp, "%s %d", macBuf, &r);
			if (res != 2) break;
			if (mac == GMac(macBuf)) {
				radioLen = r;
				qDebug() << QString("radioLen in file %1(0x%2)").arg(radioLen).arg(QString::number(radioLen, 16));
				break;
			}
		}
		fclose(fp);
	}
	return int16_t(radioLen);
}

int16_t GMonitorDevice::getRadioLenFromDevice() {
	int radioLen = -1;
	GSyncPcapDevice device;
	device.intfName_ = intfName_;
	if (!device.open()) {
		err = device.err;
		return -1;
	}

	GPacket::Dlt _dlt = device.dlt();
	if (_dlt != GPacket::Dot11) {
		QString msg = QString("Data link type(%1) must be GPacket::Dot11").arg(GPacket::dltToString(_dlt));
		SET_ERR(GErr::Fail, msg);
		return -1;
	}

	while (true) {
		GDot11Packet packet;
		GPacket::Result res = device.read(&packet);
		switch (res) {
			case GPacket::Eof:
				SET_ERR(GErr::Fail, "device.read return Eof");
				return false;
			case GPacket::Fail:
				SET_ERR(GErr::Fail, "device.read return Fail");
				return false;
			case GPacket::None:
				continue;
			case GPacket::Ok:
				break;
		}
		GRadioHdr* radioHdr = packet.radioHdr_;
		if (radioHdr == nullptr) {
			SET_ERR(GErr::ObjectIsNull, "ratioHdr is null");
			return -1;
		}
		radioLen = radioHdr->len_;
		qDebug() << QString("real radioLen %1(0x%2)").arg(radioLen).arg(QString::number(radioLen, 16));

		GIntf* intf = GNetInfo::instance().intfList().findByName(intfName_);
		if (intf == nullptr) {
			QString msg = QString("intf(%1) is null").arg(intfName_);
			SET_ERR(GErr::Fail, msg);
			return -1;
		}
		GMac mac = intf->mac();
		FILE* fp = fopen("radiolen.txt", "at");
		if (fp != nullptr) {
			fprintf(fp, "%s %d\n", qPrintable(QString(mac)), radioLen);
			fclose(fp);
		}
		break;
	}
	device.close();

	return int16_t(radioLen);
}
