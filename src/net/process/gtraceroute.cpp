#include "gtraceroute.h"

// ----------------------------------------------------------------------------
// GTraceRoute
// ----------------------------------------------------------------------------
GTraceRoute::GTraceRoute(QObject* parent) : GStateObj(parent) {
#ifndef Q_OS_ANDROID
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GIntf* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
#else
	intfName_ = "wlan0";
#endif // Q_OS_ANDROID
}

GTraceRoute::~GTraceRoute() {
}

bool GTraceRoute::doOpen() {
	if (intfName_ == "") {
		SET_ERR(GErr::InterfaceNameNotSpecified, "intfName is not specified");
		return false;
	}

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::ValueIsNull, msg);
		return false;
	}
	myIp_ = intf_->ip();

	if (!bpFilter_.open()) {
		err = bpFilter_.err;
		return false;
	}

	if (!rawIpSocketWrite_.open()) {
		err = rawIpSocketWrite_.err;
		return false;
	}

	{ QMutexLocker ml(&threadMgr_); threadMgr_.clear() ;}
	{ QMutexLocker ml(&nextProbeMgr_); nextProbeMgr_.clear() ;}

	return true;
}

bool GTraceRoute::doClose() {
	{ QMutexLocker ml(&threadMgr_); for (ProbeThread* thread: threadMgr_) thread->swe_.wakeAll(); }

	QElapsedTimer timer;
	quint64 start = timer.elapsed();
	while (true) {
		int count;
		{ QMutexLocker ml(&threadMgr_); count = threadMgr_.count(); }
		if (count == 0) break;
		qDebug() << QString("thread count %1").arg(count);

		QCoreApplication::processEvents();
		QThread::msleep(100);
		quint64 now = timer.elapsed();
		if (now - start > G::Timeout) {
			QMutexLocker ml(&threadMgr_);
			qCritical() << QString("thread count %1").arg(threadMgr_.count());
			break;
		}
	}

	bpFilter_.close();
	rawIpSocketWrite_.close();

	return true;
}

void GTraceRoute::checkTtlResponse(GPacket* packet, bool* ok) {
	GIpHdr* ipHdr = packet->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);

	GIcmpHdr* icmpHdr = packet->icmpHdr_;
	if (icmpHdr == nullptr) return;

	uint8_t type = icmpHdr->type();
	if (type != GIcmpHdr::TtlExceeded) return;
	*ok = true;

	GIcmpIpHdr* icmpIpHdr = PIcmpIpHdr(icmpHdr);
	GIpHdr* ipHdr2 = icmpIpHdr->ipHdr();
	if (!packet->buf_.contains(ipHdr2)) {
		qWarning() << QString("invalid ipHdr2 packet=%1 ipHdr2=%2").arg(QString(packet->buf_)).arg(QString::number(qintptr(ipHdr2), 16));
		return;
	}

	GIp dip2 = ipHdr2->dip();

	void* nextPdu = pbyte(ipHdr2) + ipHdr2->hlen() * 4; // tcp, udp or icmp
	if (!packet->buf_.contains(pbyte(nextPdu))) {
		qWarning() << QString("invalid nextPdu(%1) packet=%2").arg(QString::number(qintptr(nextPdu), 16)).arg(QString(packet->buf_));
		return;
	}

	uint8_t p = ipHdr2->p();
	Key key(p, dip2);
	{
		QMutexLocker ml(&threadMgr_);
		ThreadMgr::iterator it = threadMgr_.find(key);
		if (it == threadMgr_.end()) return;
		ProbeThread* probeThread = it.value();
		probeThread->checkTtlResponse(ipHdr, ipHdr2);
	}
}

void GTraceRoute::checkCreateThread(GPacket* packet) {
	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr != nullptr) {
		GMac dmac = ethHdr->dmac();
		if (dmac.isBroadcast() || dmac.isMulticast()) return;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	GIp dip = ipHdr->dip();
	if (dip == myIp_ || dip.isBroadcast() || dip.isMulticast() || dip.isLocalHost()) return;

	if (!bpFilter_.check(packet->buf_)) return;


	uint8_t p = ipHdr->p();
	Key key(p, dip);

	{
		NextProbeMgr::iterator it;
		nextProbeMgr_.deleteOldProbes(packet->ts_.tv_sec);
		it = nextProbeMgr_.find(key);
		if (it != nextProbeMgr_.end()) return;
	}

	{
		ThreadMgr::iterator it;
		{ QMutexLocker ml(&threadMgr_); it = threadMgr_.find(key); }
		if (it != threadMgr_.end()) return;
	}

	if (packet->tcpHdr_ != nullptr) {
		QMetaObject::invokeMethod(this, [this, key, packet]() {
			TcpThread* tcpThread = new TcpThread(this, packet, key);
			{ QMutexLocker ml(&threadMgr_);	threadMgr_.insert(key, tcpThread); }
			{ QMutexLocker ml(&nextProbeMgr_); nextProbeMgr_.insert(key, packet->ts_.tv_sec); }
			tcpThread->start();
		}, Qt::BlockingQueuedConnection);
		return;
	}

	if (packet->udpHdr_ != nullptr) {
		QMetaObject::invokeMethod(this, [this, key, packet]() {
			UdpThread* udpThread = new UdpThread(this, packet, key);
			{ QMutexLocker ml(&threadMgr_); threadMgr_.insert(key, udpThread); }
			{ QMutexLocker ml(&nextProbeMgr_); nextProbeMgr_.insert(key, packet->ts_.tv_sec); }
			udpThread->start();
		}, Qt::BlockingQueuedConnection);
		return;
	}

	if (packet->icmpHdr_ != nullptr) {
		QMetaObject::invokeMethod(this, [this, key, packet]() {
			IcmpThread* icmpThread = new IcmpThread(this, packet, key);
			{ QMutexLocker ml(&threadMgr_);	threadMgr_.insert(key, icmpThread); }
			{ QMutexLocker ml(&nextProbeMgr_); nextProbeMgr_.insert(key, packet->ts_.tv_sec); }
			icmpThread->start();
		}, Qt::BlockingQueuedConnection);
		return;
	}
}

void GTraceRoute::probe(GPacket* packet) {
	if (packet->ipHdr_ == nullptr) return;
	bool ok = false;
	checkTtlResponse(packet, &ok);
	if (ok) return;
	checkCreateThread(packet);
}

// ----------------------------------------------------------------------------
// ProbeThread
// ----------------------------------------------------------------------------
GTraceRoute::ProbeThread::ProbeThread(GTraceRoute* tr, GPacket* packet, Key key) : GThread(tr) {
	key_ = key;
	ipHdr_  = packet->ipHdr_;
	Q_ASSERT(ipHdr_ != nullptr);

	uint16_t tlen = ipHdr_->tlen();
	GBuf copyBuf(pbyte(ipHdr_), tlen);
	if (!packet->buf_.contains(copyBuf)) {
		qWarning() << QString("invalid packet : packet=%1 copy=%2").arg(QString(packet->buf_)).arg(QString(copyBuf));
		return;
	}

	sendPacketByteArray_.resize(copyBuf.size_);
	GIpHdr* sendIpHdr = PIpHdr(sendPacketByteArray_.data());
	memcpy(pvoid(sendIpHdr), pvoid(copyBuf.data_), copyBuf.size_);
	sendIpHdr->tos_ = 0x45;
	sendIpHdr->off_ = htons(sendIpHdr->off() | 0x8000); // set Reserved bit(MSB). means my sending ttl probe packet


	sendPacket_.clear();
	GBuf buf(pbyte(sendPacketByteArray_.data()), sendPacketByteArray_.size());
	sendPacket_.buf_ = buf;
	sendPacket_.ipHdr_ = sendIpHdr;

	logTime_ = QDateTime::fromTime_t(packet->ts_.tv_sec);
	QObject::connect(this, &QThread::finished, this, &QObject::deleteLater);
}

GTraceRoute::ProbeThread::~ProbeThread() {
	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);
	{ QMutexLocker ml(&tr->threadMgr_); tr->threadMgr_.remove(key_); }
}

void GTraceRoute::ProbeThread::run() {
	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);

	if (swe_.wait(tr->startTimeout_)) return;
	GIpHdr* ipHdr = sendPacket_.ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	for (uint8_t ttl = 1; ttl <= tr->maxHop_; ttl++) {
		ipHdr->id_ = htons(ttl);
		ipHdr->ttl_ = ttl;
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));

		for (int q = 0; q < tr->queryCount_; q++) {
			tr->rawIpSocketWrite_.write(&sendPacket_);
			swe_.wait(tr->sendTimeout_);
			if (!tr->active()) break;
		}

		if (!tr->active()) break;
		swe_.wait(tr->ttlChangeTimeout_);
	}
	if (tr->active()) swe_.wait(tr->stopTimeout_);

	writeLog();
}

bool GTraceRoute::ProbeThread::checkTtlResponse(GIpHdr* ipHdr, GIpHdr* ipHdr2) {
	if ((ipHdr2->off() & 0x8000) == 0) return false; // if not my sending ttl probe packet
	GIp hopIp = ipHdr->sip();
	int hopNo = ipHdr2->id();
	HopMap::iterator it = hopMap_.find(hopNo);
	if (it != hopMap_.end()) {
		GIp ip = it.value();
		if (ip != hopIp)
			qWarning() << QString("different ip old(%1) new(%2) hop=%3").arg(QString(ip)).arg(QString(hopIp)).arg(hopNo);
		return false;
	}
	hopMap_.insert(hopNo, hopIp);
	return true;
}

void GTraceRoute::ProbeThread::writeLog() {
	QString msg = logHeader();
	int maxHopNo = hopMap_.maxHopNo();
	for (int hopNo = 1; hopNo <= maxHopNo; hopNo++) {
		HopMap::iterator it = hopMap_.find(hopNo);
		if (it == hopMap_.end())
			msg += "*";
		else {
			GIp hopIp = it.value();
			msg += QString(hopIp);
		}
		if (hopNo < maxHopNo)
			msg += '\t';
	}

	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);

	{
		QMutexLocker ml(&tr->logFileMutex_);
		QFile file(tr->logFileName_);
		bool exists = file.exists();
		if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
			qWarning() << QString("can not open file(%1) %2").arg(tr->logFileName_).arg(file.errorString());
		else {
			QTextStream ts(&file);
			if (!exists) {
				QString header = "date\ttime\tproto\tsip\tsport\tdip\tdport\t";
				for (int i = 1; i <= tr->maxHop_; i++) {
					header += QString::number(i);
					if (i < tr->maxHop_)
						header += '\t';
				}
				ts << header << '\n';
			}
			QString dbgMsg = msg;
			dbgMsg.replace('\t', ' ');
			qDebug() << dbgMsg;
			msg = logTime_.toString("yy.MM.dd\thh:mm:ss\t") + msg + '\n';
			ts << msg;
			file.close();
		}
	}
}

// ----------------------------------------------------------------------------
// TcpThread
// ----------------------------------------------------------------------------
GTraceRoute::TcpThread::TcpThread(GTraceRoute* tr, GPacket* packet, Key key) : ProbeThread(tr, packet, key) {
	tcpHdr_ = packet->tcpHdr_;
	Q_ASSERT(tcpHdr_ != nullptr);

	sip_ = ipHdr_->sip();
	sport_ = tcpHdr_->sport();
	dip_ = ipHdr_->dip();
	dport_ = tcpHdr_->dport();

	qDebug() << QString("%1:%2 > %3:%4").arg(QString(sip_)).arg(sport_).arg(QString(dip_)).arg(dport_);
}

GTraceRoute::TcpThread::~TcpThread() {
}

// ----------------------------------------------------------------------------
// UdpThread
// ----------------------------------------------------------------------------
GTraceRoute::UdpThread::UdpThread(GTraceRoute* tr, GPacket* packet, Key key) : ProbeThread(tr, packet, key) {
	udpHdr_ = packet->udpHdr_;
	Q_ASSERT(udpHdr_ != nullptr);

	sip_ = ipHdr_->sip();
	sport_ = udpHdr_->sport();
	dip_ = ipHdr_->dip();
	dport_ = udpHdr_->dport();

	qDebug() << QString("%1:%2 > %3:%4").arg(QString(sip_)).arg(sport_).arg(QString(dip_)).arg(dport_);
}

GTraceRoute::UdpThread::~UdpThread() {
}

// ----------------------------------------------------------------------------
// IcmpThread
// ----------------------------------------------------------------------------
GTraceRoute::IcmpThread::IcmpThread(GTraceRoute* tr, GPacket* packet, Key key) : ProbeThread(tr, packet, key) {
	icmpPingHdr_ = PIcmpPingHdr(packet->icmpHdr_);
	Q_ASSERT(icmpPingHdr_ != nullptr);

	sip_ = ipHdr_->sip();
	dip_ = ipHdr_->dip();
	id_ = icmpPingHdr_->id();

	qDebug() << QString("%1 > %2 %3").arg(QString(sip_)).arg(QString(dip_)).arg(id_);
}

GTraceRoute::IcmpThread::~IcmpThread() {
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
#include "base/prop/gpropitem-filepath.h"
GPropItem* GTraceRoute::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		return new GPropItemInterface(param);
	}
	if (QString(param->mpro_.name()) == "logFileName") {
		GPropItemFilePath* res = new GPropItemFilePath(param);
		QStringList filters{"tsv files - *.tsv(*.tsv)", "any files - *(*)"};
		res->fd_->setNameFilters(filters);
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
