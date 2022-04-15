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
		SET_ERR(GErr::ObjectIsNull, "pcapDevice is null");
		return false;
	}

	if (hostDetect_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "hostDetect is null");
		return false;
	}

	et_.start();
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

	while (true) {
		int count = 0;
		{
			QMutexLocker ml(&stm_.m_);
			count = stm_.count();
		}
		if (count == 0)
			break;
		qDebug() << "thread count is" << count;
		QThread::msleep(100);
	}

	return true;
}

// ----------------------------------------------------------------------------
// GHostDelete::ScanThread
// ----------------------------------------------------------------------------
GHostDelete::ScanThread::ScanThread(GHostDelete* hostDelete, GHostDetect::Host* host) : GThread(hostDelete) {
	hd_ = hostDelete;
	host_ = host;
}

GHostDelete::ScanThread::~ScanThread() {
}

#include "net/pdu/getharphdr.h"

void GHostDelete::ScanThread::run() {
	qDebug() << "beg " << QString(host_->mac_) << QString(host_->ip_); // by gilgil 2021.11.13

	GPcapDevice* device = hd_->pcapDevice_;
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

	QElapsedTimer* et = &hd_->et_;
	bool shouldBeDeleted = false;
	while (hd_->active()) {
		if (we_.wait(hd_->checkInterval_)) {
			break;
		}

		qint64 start = et->elapsed();
		if (host_->lastAccess_ + hd_->scanStartTimeout_ < start) {
			if (hd_->randomInterval_ > 0) {
				GDuration random = QRandomGenerator::global()->generate() % hd_->randomInterval_;
				if (we_.wait(random)) break;
				qDebug() << "random" << random; // gilgil temp 2022.02.03

				bool deleted = false;
				while (true) {
					qDebug() << "arp request" << QString(host_->mac_) << QString(host_->ip_); // gilgil temp 2022.02.03
					GPacket::Result res = device->write(GBuf(pbyte(&packet), sizeof(packet)));
					if (res != GPacket::Ok) {
						qWarning() << QString("device_->write return %1").arg(int(res));
					}
					if (we_.wait(hd_->sendInterval_)) break;

					qint64 now = et->elapsed();
					if (host_->lastAccess_ + hd_->scanStartTimeout_ > start) { // detected
						qDebug() << QString("detected %1 %2").arg(QString(host_->mac_), QString(host_->ip_));
						break;
					}

					if (start + hd_->deleteTimeout_ < now) {
						shouldBeDeleted = true;
						break;
					}
				}
				if (deleted) break;
			}
		}
	}

	ScanThreadMap* stm = &hd_->stm_;
	{
		QMutexLocker ml(&stm->m_);
		int res = stm->remove(host_->mac_);
		if (res != 1) {
			qCritical() << QString("stm->remove return %1 %2 %3").arg(res).arg(QString(host_->mac_)).arg(QString(host_->ip_));
		}
	}

	QString hostMac = QString(host_->mac_);
	QString hostIp = QString(host_->ip_);
	if (shouldBeDeleted) {
		qDebug() << "deleted " + QString(host_->mac_) << QString(host_->ip_); // gilgil temp 2022.02.03
		emit hd_->hostDetect_->hostDeleted(host_);
		{
			GHostDetect::HostMap* hosts = &hd_->hostDetect_->hosts_;
			QMutexLocker ml(&hosts->m_);
			int res = hosts->remove(host_->mac_);
			if (res != 1) {
				qCritical() << QString("stm->remove return %1 %2").arg(res).arg(QString(host_->mac_));
			}
		}
	}
	qDebug() << "end" << hostMac << hostIp; // by gilgil 2021.11.13
}

void GHostDelete::processHostDetected(GHostDetect::Host* host) {
	QMutexLocker ml(&stm_.m_);
	ScanThreadMap::iterator it = stm_.find(host->mac_);
	if (it == stm_.end()) {
		QMetaObject::invokeMethod(this, [this, &host]() {
			ScanThread* ast = new ScanThread(this, host);
			QObject::connect(ast, &QThread::finished, ast, &QObject::deleteLater);
			stm_.insert(host->mac_, ast);
			ast->start();
		});
	}
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
