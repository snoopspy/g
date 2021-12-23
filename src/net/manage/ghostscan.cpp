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

	scanThread_.start();

	return true;
}

bool GHostScan::doClose() {
	if (!enabled_) return true;

	we_.wakeAll();
	bool res = scanThread_.wait();
	return res;
}

#include "net/gatm.h" // for GEthArpHdr
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
	qDebug() << QString("begIp=%1 endIp=%2").arg(QString(begIp), QString(endIp));

	GEthArpHdr packet;

	GEthHdr* ethHdr = &packet.ethHdr_;
	ethHdr->dmac_ = GMac::broadcastMac();
	ethHdr->smac_ = myMac;
	ethHdr->type_ = htons(GEthHdr::Arp);

	GArpHdr* arpHdr = &packet.arpHdr_;
	arpHdr->hrd_ = htons(GArpHdr::ETHER);
	arpHdr->pro_ = htons(GEthHdr::Ip4);
	arpHdr->hln_ = GMac::SIZE;
	arpHdr->pln_ = GIp::SIZE;
	arpHdr->op_ = htons(GArpHdr::Request);
	arpHdr->smac_ = myMac;
	arpHdr->sip_ = htonl(myIp);
	arpHdr->tmac_ = GMac::nullMac();

	GWaitEvent we;
	while (active()) {
		for (GIp ip = begIp; ip <= endIp; ip = ip + 1) {
			arpHdr->tip_ = htonl(ip);
			if (!active() || !pcapDevice_->active()) break;
			GPacket::Result res = pcapDevice_->write(GBuf(pbyte(&packet), sizeof(packet)));
			if (res != GPacket::Ok) {
				qWarning() << QString("device_->write return %1").arg(int(res));
				break;
			}
			if (we_.wait(sendSleepTime_)) break;
		}
		if (!active() || !pcapDevice_->active()) break;
		if (we_.wait(rescanSleepTime_)) break;
	}

	qDebug() << "end";
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
