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

	watchThread_.start();
	return true;
}

bool GHostWatch::doClose() {
	watchThread_.swe_.wakeAll();
	watchThread_.quit();
	bool res = watchThread_.wait();
	return res;
}

bool GHostWatch::propLoad(QJsonObject jo, QMetaProperty mpro) {
	if (QString(mpro.name()) == "pcapDevice" || QString(mpro.name()) == "hostMgr") {
		QObject* p = parent();
		if (p != nullptr && QString(p->metaObject()->className()) == "GAutoArpSpoof")
			return true;
	}
	return GStateObj::propLoad(jo, mpro);
}

#include "net/write/gpcapdevicewrite.h"

// ----------------------------------------------------------------------------
// GHostWatch::WatchThread
// ----------------------------------------------------------------------------
void GHostWatch::WatchThread::run() {
	qDebug() << "beg";
	GHostWatch* hw = dynamic_cast<GHostWatch*>(parent());
	Q_ASSERT(hw != nullptr);

	GPcapDeviceWrite deviceWrite;
	GEthArpPacket sendPacket;
	GEthHdr* ethHdr = &sendPacket.ethHdr_;
	GArpHdr* arpHdr = &sendPacket.arpHdr_;
	{
		GPcapDevice* device = hw->pcapDevice_;
		if (!device->active()) {
			qDebug() << "device->active() is false";
			return;
		}
		deviceWrite.intfName_ = device->intfName_;
		if (!deviceWrite.open()) {
			qDebug() << QString("deviceWrite.open(%1) return false").arg(deviceWrite.intfName_);
			return;
		}

		GIntf* intf = device->intf();
		Q_ASSERT(intf != nullptr);
		GIp myIp = intf->ip();
		GMac myMac = intf->mac();

		// ethHdr->dmac_ // set later
		ethHdr->smac_ = myMac;
		ethHdr->type_ = htons(GEthHdr::Arp);

		arpHdr->hrd_ = htons(GArpHdr::Ether);
		arpHdr->pro_ = htons(GEthHdr::Ip4);
		arpHdr->hln_ = GMac::Size;
		arpHdr->pln_ = GIp::Size;
		arpHdr->op_ = htons(GArpHdr::Request);
		arpHdr->smac_ = myMac;
		arpHdr->sip_ = htonl(myIp);
		arpHdr->tmac_ = GMac::nullMac();
		// arpHdr->tip_ // set later
	}

	GHostMgr::HostMap* hm = &hw->hostMgr_->hostMap_;
	class QList<SendInfo> sendInfos;

	while (true) {
		if (swe_.wait(hw->checkInterval_)) break;

		long now;
#ifdef Q_OS_WIN
		QDateTime dt = QDateTime::currentDateTime();
		now = long(dt.toSecsSinceEpoch());
#else
		struct timeval ts;
		gettimeofday(&ts, nullptr);
		now = ts.tv_sec;
#endif // Q_OS_WIN

		sendInfos.clear();
		{
			QMutexLocker ml(hm);
			for (GHostMgr::HostMap::iterator it = hm->begin(); it != hm->end(); it++) {
				GHostMgr::HostValue* hostValue = it.value();
				if (now - hostValue->lastTime_.tv_sec < hw->scanStartSec_)
					continue;
				sendInfos.push_back(SendInfo(it.key(), it.value()->ip_));
			}
		}

		bool exit = false;
		GBuf buf(pbyte(&sendPacket), sizeof(sendPacket));
		for (SendInfo& sendInfo: sendInfos) {
			ethHdr->dmac_ = sendInfo.mac_;
			arpHdr->tip_ = htonl(sendInfo.ip_);
			GPacket::Result res = deviceWrite.writeBuf(buf);
			if (res != GPacket::Ok) {
				qWarning() << QString("deviceWrite.write return %1").arg(int(res));
			}
			if (swe_.wait(hw->sendInterval_)) {
				exit = true;
				break;
			}
		}
		if (exit) break;
	}
	qDebug() << "end";
}
