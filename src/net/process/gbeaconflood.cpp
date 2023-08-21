#include "gbeaconflood.h"

// ----------------------------------------------------------------------------
// GBeaconFlood
// ----------------------------------------------------------------------------
GBeaconFlood::GBeaconFlood(QObject* parent) : GMonitorDeviceWrite(parent) {
	mtu_ = 0;
	messages_ = QStringList{
		"beaconflood0",
		"beaconflood1",
		"beaconflood2",
		"beaconflood3",
		"beaconflood4",
		"beaconflood5",
		"beaconflood6",
		"beaconflood7",
		"beaconflood8",
		"beaconflood9",
	};
}

GBeaconFlood::~GBeaconFlood() {
	close();
}

bool GBeaconFlood::doOpen() {
	if (!GMonitorDeviceWrite::doOpen()) return false;

	nextMac_ = startMac_;
	bfl_.clear();
	for (QString message: messages_) {
		if (message == "") continue;

		BeaconFrame bf;
		bf.build(message, channel_, nextMac_);
		bfl_.push_back(bf);
		nextMac_ = nextMac(nextMac_);
	}

	floodingThread_.start();
	return true;
}

bool GBeaconFlood::doClose() {
	if (!GMonitorDeviceWrite::doClose()) return false;

	floodingThread_.swe_.wakeAll();
	floodingThread_.quit();
	bool res = floodingThread_.wait();

	return res;
}

void GBeaconFlood::FloodingThread::run() {
	GBeaconFlood* beaconFlood = dynamic_cast<GBeaconFlood*>(parent());
	if (beaconFlood == nullptr) {
		qCritical() << "bf is null";
		return;
	}

	while (true) {
		bool ok = true;
		{
			QMutexLocker ml(&beaconFlood->bflMutex_);
			for (BeaconFrame& bf: beaconFlood->bfl_) {
				GBuf buf(pbyte(&bf), bf.size_);
				if (!beaconFlood->active()) break;
				GPacket::Result res = beaconFlood->write(buf);
				if (res != GPacket::Ok) {
					qWarning() << beaconFlood->err->msg();
					ok = false;
					break;
				}
				if (swe_.wait(beaconFlood->sendInterval_)) break;
			}
		}
		if (!ok || !beaconFlood->active()) break;
		if (swe_.wait(beaconFlood->interval_)) break;
	}
}

void GBeaconFlood::BeaconFrame::build(QString message, int channel, GMac mac) {
	//
	// radioHdr
	//
	radioHdr_.init();

	//
	// beaconHdr
	//
	GBeaconHdr* bh = &beaconHdr_;
	bh->ver_ = 0;
	bh->type_ = 0;
	bh->subtype_ = GDot11::Beacon;
	bh->flags_ = 0;
	bh->duration_ = 0;
	bh->addr1_ = GMac::broadcastMac();
	bh->addr2_ = mac;
	bh->addr3_ = mac;
	bh->frag_ = 0;
	bh->seq_ = 0;

	bh->fix_.timestamp_ = 0;
	bh->fix_.beaconInterval_ = 0x6400;
	bh->fix_.capabilities_ = 0x0011;

	// TagSsidParameterSet
	GBeaconHdr::Tag* tag = bh->firstTag();
	tag->num_ = GBeaconHdr::TagSsidParameterSet;
	std::string ssid = message.toStdString();
	tag->len_ = ssid.length();
	memcpy(pchar(tag->value()), ssid.data(), ssid.size());
	tag = tag->next();

	// TagSupportedRated
	tag->num_ = GBeaconHdr::TagSupportedRated;
	tag->len_ = 8;
	char* p = pchar(tag->value());
	*p++ = 0x82; // 1(B)
	*p++ = 0x84; // 2(B)
	*p++ = 0x8B; // 5.5(B)
	*p++ = 0x96; // 11(B)
	*p++ = 0x24; // 18
	*p++ = 0x30; // 24
	*p++ = 0x48; // 36
	*p++ = 0x6C; // 54
	tag = tag->next();

	// TagDsParameterSet
	tag->num_ = GBeaconHdr::TagDsParameterSet;
	tag->len_ = 1;
	*pchar(tag->value()) = channel;
	tag = tag->next();

	// TagTrafficIndicationMap
	tag->num_ = GBeaconHdr::TagTrafficIndicationMap;
	tag->len_ = sizeof(GBeaconHdr::TrafficIndicationMap) - sizeof(GBeaconHdr::Tag);
	GBeaconHdr::TrafficIndicationMap* tim = GBeaconHdr::PTrafficIndicationMap(tag);
	tim->count_ = 0;
	tim->period_ = 3;
	tim->control_ = 0;
	tim->bitmap_ = 0;
	tag = tag->next();

	// TagVendorSpecific(Microsoft Corp : WAP Information Element)
	char vendor[] = "\xdd\x18\x00\x50\xf2\x01\x01\x00\x00\x50\xf2\x04\x01\x00\x00\x50\xf2\x04\x01\x00\x00\x50\xf2\x02\x00\x00";
	memcpy(tag, vendor, sizeof(vendor) - 1);
	tag = tag->next();

	// last
	size_ = pchar(tag) - pchar(this);
}

void GBeaconFlood::addMessage(QString message) {
	QMutexLocker rml(&bflMutex_);
	BeaconFrame bf;
	bf.build(message, channel_, nextMac_);
	bfl_.push_back(bf);
}

void GBeaconFlood::removeMessage(int index) {
	QMutexLocker rml(&bflMutex_);
	bfl_.removeAt(index);
}

GMac GBeaconFlood::nextMac(GMac mac) {
	GMac res = mac;
	gbyte* p = pbyte(res) + GMac::Size - 1;
	while (true) {
		if (*p != 0xFF) {
			(*p)++;
			return res;
		}
		*p = 0x00;
		p--;
		if (p < pbyte(res)) return mac;
	}
	return res;
}
