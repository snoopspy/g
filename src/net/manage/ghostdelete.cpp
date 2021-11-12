#include "ghostdelete.h"

// ----------------------------------------------------------------------------
// GHostDelete
// ----------------------------------------------------------------------------
Q_INVOKABLE GHostDelete::GHostDelete(QObject* parent) : GStateObj(parent) {
}

GHostDelete::~GHostDelete() {
	close();
}

bool GHostDelete::doOpen() {
	if (!enabled_) return true;

	if (pcapDevice_ == nullptr) {
		SET_ERR(GErr::OBJECT_IS_NULL, "pcapDevice is null");
		return false;
	}

	if (hostDetect_ == nullptr) {
		SET_ERR(GErr::OBJECT_IS_NULL, "hostDetect is null");
		return false;
	}

	checkThread_.start();

	return true;
}

bool GHostDelete::doClose() {
	if (!enabled_) return true;

	checkThread_.we_.wakeAll();
	bool res = checkThread_.wait();
	return res;
}

void GHostDelete::checkRun() {
	qDebug() << "beg";

	QElapsedTimer et;
	while (active()) {
		{
			QMutexLocker ml(&hostDetect_->hosts_.m_);
			qint64 now = et.elapsed();
			for (GHostDetect::Host& host : hostDetect_->hosts_) {
				// qDebug() << QString("lastAccess=%1 now=%2 diff=%3").arg(host.lastAccess_).arg(now).arg(now-host.lastAccess_); // gilgil temp 2021.10.24
				if (host.lastAccess_ + scanStartTimeout_ < now) {
					QMutexLocker ml(&astm_.m_);
					ActiveScanThreadMap::iterator it = astm_.find(host.mac_);
					if (it == astm_.end()) {
						QMetaObject::invokeMethod(this, [this, &host]() {
							ActiveScanThread* ast = new ActiveScanThread(this, &host);
							QObject::connect(ast, &QThread::finished, ast, &QObject::deleteLater);
							astm_.insert(host.mac_, ast);
							ast->start();
						});
					}
				}
			}
		}
		if (checkThread_.we_.wait(checkSleepTime_)) break;
	}

	qDebug() << "end";
}

// ----------------------------------------------------------------------------
// GHostDelete::ActiveScanThread
// ----------------------------------------------------------------------------
GHostDelete::ActiveScanThread::ActiveScanThread(GHostDelete* hostDelete, GHostDetect::Host* host) : GThread(hostDelete) {
	qDebug() << "beg" << QString(host->mac_); // gilgil temp 2021.11.11
	hostDelete_ = hostDelete;
	host_ = host;
}

GHostDelete::ActiveScanThread::~ActiveScanThread() {
	qDebug() << "end"; // gilgil temp 2021.11.11
}

void GHostDelete::ActiveScanThread::run() {
	qDebug() << "beg";

	if (we_.wait(hostDelete_->randomSleepTime_)) return;

	GPcapDevice* device = hostDelete_->pcapDevice_;
	GIntf* intf = device->intf();
	Q_ASSERT(intf != nullptr);

	GIp myIp = intf->ip();
	GMac myMac = intf->mac();

	GEthArpHdr packet;

	GEthHdr* ethHdr = &packet.ethHdr_;
	ethHdr->dmac_ = host_->mac_; // Unicast
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
	arpHdr->tip_ = htonl(host_->ip_);

	QElapsedTimer et;
	qint64 start = et.elapsed();
	while (true) {
		GPacket::Result res = device->write(GBuf(pbyte(&packet), sizeof(packet)));
		if (res != GPacket::Ok) {
			qWarning() << QString("device_->write return %1 %2").arg(int(res)).arg(device->err->msg());
		}
		if (we_.wait(hostDelete_->sendSleepTime_)) break;

		qint64 now = et.elapsed();
		if (host_->lastAccess_ + hostDelete_->scanStartTimeout_ > now) { // accessed
			qDebug() << QString("access detected %1 %2").arg(QString(host_->mac_), QString(host_->ip_));
			break;
		}

		if (start + hostDelete_->deleteTimeout_ < now) {
			qDebug() << QString("%1 %2 timeout diff=%3").arg(QString(host_->mac_), QString(host_->ip_)).arg(now - start);
			emit hostDelete_->hostDeleted(host_);

			ActiveScanThreadMap* astm = &hostDelete_->astm_;
			{
				QMutexLocker ml(&hostDelete_->astm_.m_);
				qDebug() << "mac_= " << QString(host_->mac_);  // gilgil temp 2021.11.11
				int res = astm->remove(host_->mac_);
				if (res != 1) {
					qCritical() << QString("astm->remove return %1").arg(res);
				}
			}

			{
				QMutexLocker ml(&hostDelete_->hostDetect_->hosts_.m_);
				hostDelete_->hostDetect_->hosts_.remove(host_->mac_);
			}
			break;
		}
	}

	qDebug() << "end";
}

bool GHostDelete::propLoad(QJsonObject jo, QMetaProperty mpro) {
	// qDebug() << mpro.name(); // gilgil temp 2021.11.11
	if (QString(mpro.name()) == "pcapDevice" || QString(mpro.name()) == "hostDetect") {
		QObject* p = parent();
		if (p != nullptr && QString(p->metaObject()->className()) == "GAutoArpSpoof")
			return true;
	}
	return GStateObj::propLoad(jo, mpro);
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GHostDelete::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "pcapDevice" || QString(param->mpro_.name()) == "hostDetect") {
		QObject* p = parent();
		if (p != nullptr && QString(p->metaObject()->className()) == "GAutoArpSpoof")
			return nullptr;
		GPropItemInterface* res = new GPropItemInterface(param);
#ifdef Q_OS_ANDROID
		res->comboBox_->setEditable(true);
#endif
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
