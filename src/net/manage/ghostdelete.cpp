#include "ghostdelete.h"
#include <QRandomGenerator>

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

	{
		QMutexLocker ml(&stm_.m_);
		for (ScanThreadMap::iterator it = stm_.begin(); it != stm_.end(); it++) {
			ScanThread* scanThread = it.value();
			scanThread->we_.wakeAll();
		}
	}
	checkThread_.we_.wakeAll();
	qDebug() << "";
	bool res = checkThread_.wait();
	qDebug() << "";
	return res;
}

void GHostDelete::checkRun() {
	qDebug() << "beg";

	QElapsedTimer et;
	while (active()) {
		{
			GHostDetect::HostMap* hosts = &hostDetect_->hosts_;
			QMutexLocker ml(&hosts->m_);
			qint64 now = et.elapsed();
			for (GHostDetect::HostMap::iterator it = hosts->begin(); it != hosts->end();) {
				GHostDetect::Host& host = it.value();
				// qDebug() << QString("lastAccess=%1 now=%2 diff=%3").arg(host.lastAccess_).arg(now).arg(now-host.lastAccess_); // gilgil temp 2021.10.24
				if (host.deleted_ && host.lastAccess_ + hostDetect_->redetectInterval_ < now) {
					it = hosts->erase(it);
					continue;
				}
				if (host.lastAccess_ + scanStartTimeout_ < now) {
					QMutexLocker ml(&stm_.m_);
					ScanThreadMap::iterator it = stm_.find(host.mac_);
					if (it == stm_.end()) {
						QMetaObject::invokeMethod(this, [this, &host]() {
							ScanThread* ast = new ScanThread(this, &host);
							QObject::connect(ast, &QThread::finished, ast, &QObject::deleteLater);
							stm_.insert(host.mac_, ast);
							ast->start();
						});
					}
				}
				it++;
			}
		}
		if (checkThread_.we_.wait(checkSleepTime_)) break;
	}

	qDebug() << "end";
}

// ----------------------------------------------------------------------------
// GHostDelete::ScanThread
// ----------------------------------------------------------------------------
GHostDelete::ScanThread::ScanThread(GHostDelete* hostDelete, GHostDetect::Host* host) : GThread(hostDelete) {
	hostDelete_ = hostDelete;
	host_ = host;
}

GHostDelete::ScanThread::~ScanThread() {
}

void GHostDelete::ScanThread::run() {
	// qDebug() << "beg " + QString(host_->mac_);  // by gilgil 2021.11.13
	GDuration random = QRandomGenerator::global()->generate() % hostDelete_->randomSleepTime_;
	if (we_.wait(random)) return;
	qDebug() << QString("aft we_.wait(%1)").arg(random);

	GPcapDevice* device = hostDelete_->pcapDevice_;
	if (!device->active()) {
		qDebug() << "device->active() is false";
		return;
	}
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

	bool shouldBeDeleted = true;
	while (true) {
		GPacket::Result res = device->write(GBuf(pbyte(&packet), sizeof(packet)));
		if (res != GPacket::Ok) {
			qWarning() << QString("device_->write return %1").arg(int(res));
		}
		if (we_.wait(hostDelete_->sendSleepTime_)) break;
		if (!device->active() || !hostDelete_->active()) break;

		qint64 now = et.elapsed();
		if (host_->lastAccess_ + hostDelete_->scanStartTimeout_ > now) { // accessed
			qDebug() << QString("detect %1 %2").arg(QString(host_->mac_), QString(host_->ip_));
			shouldBeDeleted = false;
			break;
		}

		if (start + hostDelete_->deleteTimeout_ < now) {
			qDebug() << QString("timeout %1 %2 diff=%3").arg(QString(host_->mac_), QString(host_->ip_)).arg(now - start);
			emit hostDelete_->hostDetect_->hostDeleted(host_);
			break;
		}
	}

	ScanThreadMap* astm = &hostDelete_->stm_;
	{
		QMutexLocker ml(&astm->m_);
		int res = astm->remove(host_->mac_);
		if (res != 1) {
			qCritical() << QString("astm->remove return %1").arg(res);
		}
	}

	if (shouldBeDeleted)
		host_->deleted_ = true;
	// qDebug() << "end " + QString(host_->mac_);  // by gilgil 2021.11.13
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
