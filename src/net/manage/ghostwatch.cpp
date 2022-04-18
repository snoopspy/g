#include "ghostwatch.h"
#include "net/pdu/getharppacket.h"

// ----------------------------------------------------------------------------
// GHostWatch
// ----------------------------------------------------------------------------
GHostWatch::GHostWatch(QObject* parent) : GStateObj(parent) {
}

GHostWatch::~GHostWatch() {
	close();
}

bool GHostWatch::doOpen() {
	if (!enabled_) return true;

	if (pcapDevice_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "pcapDevice is null");
		return false;
	}

	if (hostMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "hostMgr is null");
		return false;
	}

	hostOffset_ = hostMgr_->requestItems_.request("GHostWatch-host", sizeof(Item));
	hostMgr_->managables_.insert(this);
	et_.start();
	threadCount_ = 0;
	return true;
}

bool GHostWatch::doClose() {
	if (threadCount_ != 0) {
		qWarning() << "threadCount is " << threadCount_;
	}
	return true;
}

void GHostWatch::hostDetected(GMac mac, GHostMgr::Value* value) {
	Item* item = PItem(value->mem(hostOffset_));
	new (item) Item(this, mac, value);
}

void GHostWatch::hostDeleted(GMac mac, GHostMgr::Value* value) {
	(void)mac;
	Item* item = PItem(value->mem(hostOffset_));
	item->~Item();
}

bool GHostWatch::propLoad(QJsonObject jo, QMetaProperty mpro)  {
	// qDebug() << mpro.name(); // gilgil temp 2021.11.11
	if (QString(mpro.name()) == "pcapDevice" || QString(mpro.name()) == "hostDetect") {
		QObject* p = parent();
		if (p != nullptr && QString(p->metaObject()->className()) == "GAutoArpSpoof")
			return true;
	}
	return GStateObj::propLoad(jo, mpro);
}

// ----------------------------------------------------------------------------
// GHostWatch::WatchThread
// ----------------------------------------------------------------------------
GHostWatch::WatchThread::WatchThread(GHostWatch* hostWatch, GMac mac, GHostMgr::Value* value) : hw_(hostWatch), mac_(mac), value_(value) {
}

GHostWatch::WatchThread::~WatchThread() {

}

#include "net/pdu/getharppacket.h"
#include "net/write/gpcapdevicewrite.h"

void GHostWatch::WatchThread::run() {
	GMac mac = mac_;
	GIp ip = value_->ip_;
	qDebug() << "beg " << QString(mac) << QString(ip); // by gilgil 2022.03.02

	GPcapDevice* device = hw_->pcapDevice_;
	if (!device->active()) {
		qDebug() << "device->active() is false";
		return;
	}
	GIntf* intf = device->intf();
	Q_ASSERT(intf != nullptr);

	GIp myIp = intf->ip();
	GMac myMac = intf->mac();

	GPcapDeviceWrite deviceWrite;
	deviceWrite.intfName_ = device->intfName_;
	if (!deviceWrite.open()) {
		qDebug() << QString("deviceWrite.open(%1) return false").arg(deviceWrite.intfName_);
		return;
	}

	GEthArpPacket packet;

	GEthHdr* ethHdr = &packet.ethHdr_;
	ethHdr->dmac_ = mac; // Unicast
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
	arpHdr->tip_ = htonl(ip);

	QElapsedTimer* et = &hw_->et_;
	qint64 start = et->elapsed();
	__time_t startEpoch = value_->ts_.tv_sec;
	while (device->active() && hw_->active()) {
		if (we_.wait(hw_->checkInterval_))
			break;

		qint64 now = et->elapsed();
		if (value_->ts_.tv_sec + hw_->scanStartTimeoutSec_ <= startEpoch + (now - start) / 1000) {
			if (hw_->randomInterval_ > 0) {
				GDuration random = QRandomGenerator::global()->generate() % hw_->randomInterval_;
				if (we_.wait(random))
					break;
			}

			bool exit = false;
			while (true) {
				qDebug() << "arp request" << QString(mac) << QString(ip); // gilgil temp 2022.02.03
				GPacket::Result res = deviceWrite.write(GBuf(pbyte(&packet), sizeof(packet)));
				if (res != GPacket::Ok) {
					qWarning() << QString("deviceWrite.write return %1").arg(int(res));
				}
				if (we_.wait(hw_->sendInterval_)) {
					exit = true;
					break;
				}

				if (value_->ts_.tv_sec + hw_->scanStartTimeoutSec_ > startEpoch + (now - start) / 1000) { // detected
					qDebug() << QString("detected %1 %2").arg(QString(mac), QString(ip));
					break;
				}
			}
			if (exit) break;
		}
	}
	qDebug() << "end " << QString(mac) << QString(ip); // by gilgil 2022.03.02
}
