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
		QString lengthFilter = QString("radio[0:4]==%1 && radio[4:4]=%2").arg((radioInfo_.radioHdr64_ & 0xFFFFFFFF00000000) >> 32).arg(radioInfo_.radioHdr64_ & 0x00000000FFFFFFFF);
		if (filter_ == "")
			filter_ = lengthFilter;
		else
			filter_ = QString("(%1) and (%2)").arg(filter_, lengthFilter);

		qDebug() << objectName();
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
	GMac myMac = intf->mac();
	QFile file("radioinfo.txt");
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return false;
	}
	QTextStream ts(&file);
	QString s;
	while (ts.readLineInto(&s)) {
		QStringList sl = s.split(" ");
		if (sl.count() != 3) continue;
		GMac mac = GMac(sl.at(0));
		uint64_t radioHdr64 = sl.at(1).toULongLong(nullptr, 16);
		uint32_t fcsSize = sl.at(2).toULong();
		if (mac == myMac) {
			qDebug() << QString("file radioInfo radioHdr=0x%1 fcsSize=%2").arg(QString::number(radioHdr64, 16)).arg(fcsSize);
			radioInfo_.radioHdr64_ = radioHdr64;
			radioInfo_.fcsSize_ = fcsSize;
			return true;
		}
	}
	return false;
}

bool GMonitorDevice::getRadioInfoFromDevice() {
	GSyncPcapDevice device;
#ifdef Q_OS_ANDROID
	device.port_ = GDemon::NexmonPort;
#endif // Q_OS_ANDROID
	if (objectName() != "") device.setObjectName(objectName() + ".device");
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
	iw.setChannel(1);
	iw.close();

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

		uint64_t radioHdr64 = be64toh(*reinterpret_cast<uint64_t*>(radioHdr));
		uint32_t fcsSize = radioHdr->getFcsSize();

		qDebug() << QString("device radioInfo radioHdr=0x%1 fcsSize=%2").arg(QString::number(radioHdr64, 16)).arg(fcsSize);
		radioInfo_.radioHdr64_ = radioHdr64;
		radioInfo_.fcsSize_ = fcsSize;

		GIntf* intf = GNetInfo::instance().intfList().findByName(intfName_);
		if (intf == nullptr) {
			QString msg = QString("intf(%1) is null").arg(intfName_);
			SET_ERR(GErr::Fail, msg);
			device.close();
			return false;
		}
		GMac myMac = intf->mac();
		QFile file("radioinfo.txt");
		if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Append)) {
			qDebug() << QString("can not open file %1").arg(file.fileName());
			return false;
		}
		QTextStream ts(&file);
		ts << QString(myMac) << " " << QString::number(radioHdr64, 16) << " " << fcsSize << "\n";
		file.close();
		break;
	}
	device.close();

	return true;
}
