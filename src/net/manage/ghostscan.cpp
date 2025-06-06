#include "ghostscan.h"

// ----------------------------------------------------------------------------
// GHostScan
// ----------------------------------------------------------------------------
GHostScan::GHostScan(QObject* parent) : GStateObj(parent) {
}

GHostScan::~GHostScan() {
	close();
}

bool GHostScan::doOpen() {
	if (!enabled_) return true;

	if (pcapDevice_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "pcapDevice is null");
		return false;
	}

	GIntf* intf = pcapDevice_->intf();
	if (intf == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "intf is null");
		return false;
	}

	if (intf->ip() == 0) {
		SET_ERR(GErr::ValueIsNotNull, "ip is zero");
		return false;
	}

	swe_.init();
	scanThread_.start();

	return true;
}

bool GHostScan::doClose() {
	if (!enabled_) return true;

	swe_.wakeAll();
	scanThread_.quit();
	bool res = scanThread_.wait();
	return res;
}

bool GHostScan::propLoad(QJsonObject jo, QMetaProperty mpro) {
	// qDebug() << mpro.name(); // gilgil temp 2021.11.11
	if (QString(mpro.name()) == "pcapDevice") {
		QObject* p = parent();
		if (p != nullptr && QString(p->metaObject()->className()) == "GAutoArpSpoof")
			return true;
	}
	return GStateObj::propLoad(jo, mpro);
}

#include "net/pdu/getharppacket.h"

void GHostScan::run() {
	qDebug() << "beg";

	GIntf* intf = pcapDevice_->intf();
	if (intf == nullptr) {
		qCritical() << "intf is null";
		return;
	}

	GIp myIp = intf->ip();
	GMac myMac = intf->mac();

	GIp begIp = intf->getBeginIp();
	GIp endIp = intf->getEndIp();
	GIp scanNetmask = scanNetmask_.toUInt(nullptr, 16);
	GIp mask = intf->mask();
	if (mask < scanNetmask)
		mask = scanNetmask;
	begIp = (myIp & mask) + 1;
	endIp = myIp | ~mask;
	qDebug() << QString("begIp=%1 endIp=%2").arg(QString(begIp), QString(endIp));

	GEthArpPacket packet;

	GEthHdr* ethHdr = &packet.ethHdr_;
	ethHdr->dmac_ = GMac::broadcastMac();
	ethHdr->smac_ = myMac;
	ethHdr->type_ = htons(GEthHdr::Arp);

	GArpHdr* arpHdr = &packet.arpHdr_;
	arpHdr->hrd_ = htons(GArpHdr::Ether);
	arpHdr->pro_ = htons(GEthHdr::Ip4);
	arpHdr->hln_ = GMac::Size;
	arpHdr->pln_ = GIp::Size;
	arpHdr->op_ = htons(GArpHdr::Request);
	arpHdr->smac_ = myMac;
	arpHdr->sip_ = htonl(myIp);
	arpHdr->tmac_ = GMac::nullMac();

	bool exit = false;
	while (true) {
		for (int cnt = 0; cnt < scanCount_; cnt++) {
			for (GIp ip = begIp; ip < endIp; ip = ip + 1) {
				arpHdr->tip_ = htonl(ip);
				if (!active() || !pcapDevice_->active()) {
					exit = true;
					break;
				}
				GPacket::Result res = pcapDevice_->writeBuf(GBuf(pbyte(&packet), sizeof(packet)));
				if (res != GPacket::Ok) {
					qWarning() << QString("device_->write return %1").arg(int(res));
					exit = true;
					break;
				}
				if (exit) break;
				if (swe_.wait(sendInterval_)) {
					exit = true;
					break;
				}
			}
			if (exit) break;
		}
		if (exit) break;
		if (swe_.wait(rescanInterval_)) {
			exit = true;
			break;
		}
	}
	qDebug() << "end";
}
