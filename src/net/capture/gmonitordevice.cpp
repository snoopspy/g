#include "gmonitordevice.h"
#include "gsyncpcapdevice.h"
#include "net/packet/gdot11packet.h"

GMonitorDevice::GMonitorDevice(QObject* parent) : GPcapDevice(parent) {
}

GMonitorDevice::~GMonitorDevice() {
	close();
}

bool GMonitorDevice::doOpen() {
	GSyncPcapDevice device;
	device.intfName_ = intfName_;
	if (!device.open()) {
		err = device.err;
		return false;
	}

	GPacket::Dlt dlt = device.dlt();
	if (dlt != GPacket::Dot11) {
		QString msg = QString("data link type is not do11 %1").arg(GPacket::dltToString(dlt));
		SET_ERR(GErr::Fail, msg);
		return false;
	}

	le16_t radioLen = 0;
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
		GRadiotapHdr* radiotapHdr = packet.radiotapHdr_;
		if (radiotapHdr == nullptr) {
			SET_ERR(GErr::ObjectIsNull, "ratiotapHdr is null");
			return false;
		}
		radioLen = radiotapHdr->len_;
		qDebug() << "radioLen=" << radioLen; // gilgil temp 2022.01.11
		break;
	}
	device.close();

	QString backupFilter = filter_;
	radioLen = htons(radioLen);
	QString lengthFilter = QString("radio[2:2]==%1").arg(radioLen);
	if (filter_ == "")
		filter_ = lengthFilter;
	else
		filter_ = QString("(%1) and (%2)").arg(filter_).arg(lengthFilter);

	bool res = GPcapDevice::doOpen();
	filter_ = backupFilter;

	return res;
}

bool GMonitorDevice::doClose() {
	return GPcapDevice::doClose();
}
