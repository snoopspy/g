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
	watchThread_.we_.wakeAll();
	watchThread_.quit();
	bool res = watchThread_.wait();
	return res;
}

bool GHostWatch::propLoad(QJsonObject jo, QMetaProperty mpro)  {
	// qDebug() << mpro.name(); // gilgil temp 2021.11.11
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
	qDebug() << "";
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

		arpHdr->hrd_ = htons(GArpHdr::ETHER);
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
	class QVector<SendInfo> sendInfos;
	struct timeval start;
	gettimeofday(&start, nullptr);
	while (true) {
		if (we_.wait(hw->checkInterval_)) break;

		struct timeval now;
		gettimeofday(&now, nullptr);

		sendInfos.clear();
		{
			QMutexLocker ml(&hm->m_);
			for (GHostMgr::HostMap::iterator it = hm->begin(); it != hm->end(); it++) {
				GHostMgr::HostValue* hostValue = it.value();
				if (now.tv_sec - hostValue->ts_.tv_sec <= hw->scanTimeoutSec_)
					continue;
				sendInfos.push_back(SendInfo(it.key(), it.value()->ip_));
			}
		}

		bool exit = false;
		GBuf buf(pbyte(&sendPacket), sizeof(sendPacket));
		for (SendInfo& sendInfo: sendInfos) {
			ethHdr->dmac_ = sendInfo.mac_;
			arpHdr->tip_ = htonl(sendInfo.ip_);
			GPacket::Result res = deviceWrite.write(buf);
			if (res != GPacket::Ok) {
				qWarning() << QString("deviceWrite.write return %1").arg(int(res));
			}
			if (we_.wait(hw->sendInterval_)) {
				exit = true;
				break;
			}
		}
		if (exit) break;
	}
	qDebug() << "";
}
