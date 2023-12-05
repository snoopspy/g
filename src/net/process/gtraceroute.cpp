#include "gtraceroute.h"

// ----------------------------------------------------------------------------
// GTraceRoute
// ----------------------------------------------------------------------------
GTraceRoute::GTraceRoute(QObject* parent) : GStateObj(parent) {
	qDebug() << "";
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
	qDebug() << "";
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

	if (rawIpSocketWrite_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "rawIpSocketWrite is null");
		return false;
	}

	return true;
}

bool GTraceRoute::doClose() {
	qDebug() << "";

	{ QMutexLocker ml(&tcpMgr_); for (TcpThread* tcpThread: tcpMgr_) tcpThread->swe_.wakeAll(); }
	{ QMutexLocker ml(&udpMgr_); for (UdpThread* udpThread: udpMgr_) udpThread->swe_.wakeAll(); }
	{ QMutexLocker ml(&icmpMgr_); for (IcmpThread* icmpThread: icmpMgr_) icmpThread->swe_.wakeAll(); }

	QElapsedTimer timer;
	quint64 start = timer.elapsed();
	while (true) {
		int tcpCount, udpCount, icmpCount;
		{ QMutexLocker ml(&tcpMgr_); tcpCount = tcpMgr_.count(); }
		{ QMutexLocker ml(&udpMgr_); udpCount = udpMgr_.count(); }
		{ QMutexLocker ml(&icmpMgr_); icmpCount = icmpMgr_.count();	}
		if (tcpCount == 0 && udpCount == 0 && icmpCount == 0) break;
		{
			QMutexLocker ml1(&tcpMgr_);
			QMutexLocker ml2(&udpMgr_);
			QMutexLocker ml3(&icmpMgr_);
			qDebug() << QString("thread count %1 %2 %3").arg(tcpMgr_.count()).arg(tcpMgr_.count()).arg(tcpMgr_.count());
		}

		QCoreApplication::processEvents();
		QThread::msleep(100);
		quint64 now = timer.elapsed();
		if (now - start > G::Timeout) {
			QMutexLocker ml1(&tcpMgr_);
			QMutexLocker ml2(&udpMgr_);
			QMutexLocker ml3(&icmpMgr_);
			qCritical() << QString("thread count %1 %2 %3").arg(tcpMgr_.count()).arg(tcpMgr_.count()).arg(tcpMgr_.count());
			break;
		}
	}
	return true;
}

void GTraceRoute::processHostResponse(GPacket* packet, bool* ok) {
	GIpHdr* ipHdr = packet->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);

	GIp sip = ipHdr->sip();
	GIp dip = ipHdr->dip();

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr != nullptr) {
		uint8_t flags = tcpHdr->flags();
		if (flags != (GTcpHdr::Syn | GTcpHdr::Ack)) return;
		uint16_t sport = tcpHdr->sport();
		// qDebug() << QString("%1 %2 %3").arg(QString(sip)).arg(QString(dip)).arg(sport); // gilgil temp 2023.12.05
		TcpKey tcpKey(dip, sip, sport);
		{
			QMutexLocker ml(&tcpMgr_);
			TcpMgr::iterator it = tcpMgr_.find(tcpKey);
			if (it == tcpMgr_.end()) return;
			TcpThread* tcpThread = it.value();
			*ok = tcpThread->processHostResponse(tcpHdr);
		}
		return;
	}

	GUdpHdr* udpHdr = packet->udpHdr_;
	if (udpHdr != nullptr) {
		qDebug() << "udp later";
		return;
	}

	GIcmpHdr* icmpHdr = packet->icmpHdr_;
	if (icmpHdr != nullptr) {
		uint8_t type = icmpHdr->type();
		if (type != GIcmpHdr::PingReply) return;
		GIcmpPingHdr* icmpPingHdr = PIcmpPingHdr(icmpHdr);
		uint16_t id = icmpPingHdr->id();
		IcmpKey icmpKey(dip, sip);
		{
			QMutexLocker ml(&icmpMgr_);
			IcmpMgr::iterator it = icmpMgr_.find(icmpKey);
			if (it != icmpMgr_.end()) {
				IcmpThread* icmpThread = it.value();
				*ok = icmpThread->processHostResponse(icmpPingHdr);
			}
		}
		return;
	}
}

void GTraceRoute::processTtlResponse(GPacket* packet, bool* ok) {
	GIpHdr* ipHdr = packet->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);

	GIcmpHdr* icmpHdr = packet->icmpHdr_;
	if (icmpHdr == nullptr) return;

	uint8_t type = icmpHdr->type();
	if (type != GIcmpHdr::TtlExceeded) return;

	GIp sip = ipHdr->sip();
	GIp dip = ipHdr->dip();

	gbyte* end = packet->buf_.data_ + packet->buf_.size_;
	GIcmpIpHdr* icmpIpHdr = PIcmpIpHdr(icmpHdr);
	GIpHdr* ipHdr2 = icmpIpHdr->ipHdr();
	if (pbyte(ipHdr2) + sizeof(GIpHdr) > pbyte(end)) {
		qWarning() << QString("invalid packet data=0x%1 size=%2 ipHdr2=0x%3")
			.arg(QString::number(qintptr(packet->buf_.data_), 16)).arg(packet->buf_.size_).arg(QString::number(qintptr(ipHdr2), 16));
		return;
	}

	GIp sip2 = ipHdr2->sip();
	GIp dip2 = ipHdr2->dip();

	void* nextPdu = pbyte(ipHdr2) + ipHdr2->hlen() *4;
	uint8_t p = ipHdr2->p();
	switch (p) {
		case GIpHdr::Tcp : {
			GTcpHdr* tcpHdr2 = PTcpHdr(nextPdu);
			if (pbyte(tcpHdr2) + 8 > pbyte(end)) { // 8(sport + dport + seq size)
				qWarning() << QString("invalid packet data=0x%1 size=%2 tcpHdr2=0x%3")
					.arg(QString::number(qintptr(packet->buf_.data_), 16)).arg(packet->buf_.size_).arg(QString::number(qintptr(tcpHdr2), 16));
				return;
			}
			uint16_t dport2 = tcpHdr2->dport();
			TcpKey tcpKey(sip2, dip2, dport2);
			{
				QMutexLocker ml(&tcpMgr_);
				TcpMgr::iterator it = tcpMgr_.find(tcpKey);
				if (it == tcpMgr_.end()) return;
				TcpThread* tcpThread = it.value();
				*ok = tcpThread->processTtlResponse(ipHdr, ipHdr2, tcpHdr2);
				return;
			}
		}
		default: break;
	}
}

void GTraceRoute::processCreateThread(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);

	GIp sip = ipHdr->sip();
	GIp dip = ipHdr->dip();
	if (dip == myIp_) return;

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr != nullptr) {
		uint8_t flags = tcpHdr->flags();
		if (flags != GTcpHdr::Syn) return;
		uint16_t port = tcpHdr->dport();
		TcpKey tcpKey(sip, dip, port);
		TcpMgr::iterator it = tcpMgr_.find(tcpKey);
		if (it == tcpMgr_.end()) {
			QMetaObject::invokeMethod(this, [this, tcpKey, packet]() {
				TcpThread* tcpThread = new TcpThread(this, tcpKey, packet);
				tcpThread->start();
			}, Qt::BlockingQueuedConnection);
		}
		return;
	}

	GUdpHdr* udpHdr = packet->udpHdr_;
	if (udpHdr != nullptr) {
		uint16_t port = udpHdr->dport();
		UdpKey udpKey(sip, dip, port);
		UdpMgr::iterator it = udpMgr_.find(udpKey);
		if (it == udpMgr_.end()) {
			QMetaObject::invokeMethod(this, [this, udpKey, packet]() {
				UdpThread* udpThread = new UdpThread(this, udpKey, packet);
				udpThread->start();
			}, Qt::BlockingQueuedConnection);
			return;
		}
	}

	GIcmpHdr* icmpHdr = packet->icmpHdr_;
	if (icmpHdr != nullptr) {
		uint8_t type = icmpHdr->type();
		if (type != GIcmpHdr::PingRequest) return;
		IcmpKey icmpKey(sip, dip);
		IcmpMgr::iterator it = icmpMgr_.find(icmpKey);
		if (it == icmpMgr_.end()) {
			QMetaObject::invokeMethod(this, [this, icmpKey, packet]() {
				IcmpThread* icmpThread = new IcmpThread(this, icmpKey, packet);
				icmpThread->start();
			}, Qt::BlockingQueuedConnection);
			return;
		}
	}
}

void GTraceRoute::probe(GPacket* packet) {
	if (packet->ipHdr_ == nullptr) return;
	bool ok = false;
	processHostResponse(packet, &ok);
	if (ok) return;
	processTtlResponse(packet, &ok);
	if (ok) return;
	processCreateThread(packet);
}

// ----------------------------------------------------------------------------
// ProbeThread
// ----------------------------------------------------------------------------
GTraceRoute::ProbeThread::ProbeThread(GTraceRoute* tr) : GThread(tr) {
	QObject::connect(this, &QThread::finished, this, &QObject::deleteLater);
}

GTraceRoute::ProbeThread::~ProbeThread() {
}

void GTraceRoute::ProbeThread::writeLog() {
	QString msg = logHeader();
	int maxHopNo = std::max(hopMap_.maxHopNo(), hostHopNo_);
	qDebug() << "maxHopNo is" << maxHopNo; // gilgil temp 2023.12.06
	for (int hopNo = 1; hopNo <= maxHopNo; hopNo++) {
		if (hopNo == hostHopNo_) {
			msg += QString(hostHopIp_);
		} else {
			HopMap::iterator it = hopMap_.find(hopNo);
			if (it == hopMap_.end())
				msg += "*";
			else {
				GIp hopIp = it.value();
				msg += QString(hopIp);
			}
		}
		if (hopNo < maxHopNo)
			msg += '\t';
	}
	msg += '\n';

	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);

	QFile file(tr->logFileName_);
	bool exists = file.exists();
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
		qWarning() << QString("can not open file(%1) %2").arg(tr->logFileName_).arg(file.errorString());
	else {
		QTextStream ts(&file);
		if (!exists) {
			QString header = "proto\tsrc\tdst\t";
			for (int i = 1; i <= tr->maxHop_; i++) {
				header += QString::number(i);
				if (i < tr->maxHop_)
					header += '\t';
			}
			ts << header << '\n';
		}
		ts << msg;
		file.close();
	}
}

// ----------------------------------------------------------------------------
// TcpThread
// ----------------------------------------------------------------------------
GTraceRoute::TcpThread::TcpThread(GTraceRoute* tr, TcpKey tcpKey, GPacket* packet) : ProbeThread(tr) {
	qDebug() << QString("%1 %2 %3").arg(QString(tcpKey.sip_)).arg(QString(tcpKey.dip_)).arg(tcpKey.port_);

	tcpKey_ = tcpKey;

	GIpHdr* ipHdr = packet->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	GTcpHdr* tcpHdr = packet->tcpHdr_;
	Q_ASSERT(tcpHdr != nullptr);

	sendPacketByteArray_.resize(sizeof(GIpHdr) + sizeof(GTcpHdr));
	GIpHdr* sendIpHdr = PIpHdr(sendPacketByteArray_.data());
	GTcpHdr* sendTcpHdr = PTcpHdr(sendPacketByteArray_.data() + sizeof(GIpHdr));

	*sendIpHdr = *ipHdr;
	sendIpHdr->tlen_ = htons(sizeof(GIpHdr) + sizeof(GTcpHdr));

	*sendTcpHdr = *tcpHdr;
	uint16_t sport = tr->tcpLocalPort_;
	if (sport == 0)
		sport = (rand() % 65535) | 0x8000;
	sport_ = sport;
	sendTcpHdr->sport_ = htons(sport);
	sendTcpHdr->seq_ = 0;
	sendTcpHdr->ack_ = 0;
	sendTcpHdr->off_rsvd_ = 0x50; // length is 20
	sendTcpHdr->flags_ = GTcpHdr::Syn;
	sendTcpHdr->win_ = htons(0x1234);
	sendTcpHdr->urp_ = 0;
	sendTcpHdr->sum_ = htons(GTcpHdr::calcChecksum(sendIpHdr, sendTcpHdr));

	sendPacket_.clear();
	GBuf buf(pbyte(sendPacketByteArray_.data()), sendPacketByteArray_.size());
	sendPacket_.buf_ = buf;
	sendPacket_.ipHdr_ = sendIpHdr;
	sendPacket_.tcpHdr_ = sendTcpHdr;

	hostHopIp_ = tcpKey.dip_;

	{
		QMutexLocker ml(&tr->tcpMgr_);
		tr->tcpMgr_.insert(tcpKey, this);
	}
}

GTraceRoute::TcpThread::~TcpThread() {
	// GDEBUG_DTOR
	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);
	{
		QMutexLocker ml(&tr->tcpMgr_);
		tr->tcpMgr_.remove(tcpKey_);
	}
}

void GTraceRoute::TcpThread::run() {
	qDebug() << "beg";
	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);

	if (swe_.wait(tr->startTimeout_)) return;
	GIpHdr* ipHdr = sendPacket_.ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	GTcpHdr* tcpHdr = sendPacket_.tcpHdr_;
	uint16_t oldTcpChecksum = tcpHdr->sum();
	Q_ASSERT(tcpHdr != nullptr);
	for (uint8_t ttl = 1; ttl <= tr->maxHop_; ttl++) {
		ipHdr->tos_ = 0x44;
		ipHdr->id_ = htons(ttl);
		ipHdr->ttl_ = ttl;
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));

		tcpHdr->seq_ = htonl(ttl);
		tcpHdr->sum_ = htons(GIpHdr::recalcChecksum(oldTcpChecksum, uint16_t(0), uint16_t(ttl)));

		for (int q = 0; q < tr->queryCount_; q++) {
			tr->rawIpSocketWrite_->write(&sendPacket_);
			swe_.wait(tr->sendTimeout_);
			if (!tr->active()) break;
		}

		if (!tr->active()) break;
		swe_.wait(tr->ttlChangeTimeout_);
	}
	if (tr->active()) swe_.wait(tr->stopTimeout_);

	writeLog();
	qDebug() << "end";
}

bool GTraceRoute::TcpThread::processHostResponse(GTcpHdr* tcpHdr) {
	uint16_t dport = tcpHdr->dport();
	if (dport != sport_) return false;
	uint32_t ack = tcpHdr->ack();
	int hopNo = ack - 1; // seq number sent previously
	if (hostHopNo_ != 0 && hopNo >= hostHopNo_) return false;
	hostHopNo_ = hopNo;
	GIp hopIp = tcpKey_.dip_;
	qDebug() << QString("%1 %2").arg(hopNo).arg(QString(hopIp));
	return true;
}

bool GTraceRoute::TcpThread::processTtlResponse(GIpHdr* ipHdr, GIpHdr* ipHdr2, GTcpHdr* tcpHdr2) {
	uint16_t id = ipHdr2->id();
	uint32_t seq = tcpHdr2->seq();
	if (id != seq) {
		qWarning() << QString("different id(%1) seq(%2)").arg(id).arg(seq);
		return false;
	}
	int hopNo = id;
	GIp hopIp = ipHdr->sip();
	HopMap::iterator it = hopMap_.find(hopNo);
	if (it != hopMap_.end()) {
		GIp ip = it.value();
		if (ip != hopIp) {
			qWarning() << QString("different ip old(%1) new(%2)").arg(QString(ip)).arg(QString(hopIp));
			return false;
		}
	}
	if (hostHopNo_ != 0 && hopNo == hostHopNo_)
		qWarning() << QString("same hopNo and hostHopNo(%1)").arg(hopIp);
	hopMap_.insert(hopNo, hopIp);
	qDebug() << QString("%1 %2").arg(hopNo).arg(QString(hopIp));
	return true;
}

// ----------------------------------------------------------------------------
// UdpThread
// ----------------------------------------------------------------------------
GTraceRoute::UdpThread::UdpThread(GTraceRoute* tr, UdpKey udpKey, GPacket* packet) : ProbeThread(tr) {
	qDebug() << "";
	(void)udpKey;
	(void)packet;
}

GTraceRoute::UdpThread::~UdpThread() {
	qDebug() << "";
}

void GTraceRoute::UdpThread::run() {
	qDebug() << "";
}

// ----------------------------------------------------------------------------
// IcmpThread
// ----------------------------------------------------------------------------
GTraceRoute::IcmpThread::IcmpThread(GTraceRoute* tr, IcmpKey icmpKey, GPacket* packet) : ProbeThread(tr) {
	qDebug() << QString("%1 %2").arg(QString(icmpKey.sip_)).arg(QString(icmpKey.dip_));
	icmpKey_ = icmpKey;

	GIpHdr* ipHdr = packet->ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	GIcmpHdr* icmpHdr = packet->icmpHdr_;
	Q_ASSERT(icmpHdr != nullptr);

	uint8_t type = icmpHdr->type();
	Q_ASSERT(type == GIcmpHdr::PingRequest);
	GIcmpPingHdr* icmpPingHdr = PIcmpPingHdr(icmpHdr);
	gbyte* icmpData = icmpPingHdr->data();
	size_t icmpDataLen = ipHdr->tlen() - (ipHdr->hlen() * 4 + sizeof(GIcmpPingHdr));
	qDebug() << "icmpDataLen =" << icmpDataLen << sizeof(GIcmpPingHdr) << sizeof(struct timeval); // gilgil temp 2023..12.06

	sendPacketByteArray_.resize(sizeof(GIpHdr) + sizeof(GIcmpPingHdr) + icmpDataLen);
	GIpHdr* sendIpHdr = PIpHdr(sendPacketByteArray_.data());
	GIcmpPingHdr* sendIcmpPingHdr = PIcmpPingHdr(sendPacketByteArray_.data() + sizeof(GIpHdr));
	gbyte* sendIcmpData = pbyte(sendPacketByteArray_.data() + sizeof(GIpHdr) + sizeof(GIcmpPingHdr));

	*sendIpHdr = *ipHdr;

	*sendIcmpPingHdr = *icmpPingHdr;
	uint16_t id = tr->icmpId_;
	if (id == 0)
		id = (rand() % 65535) | 0x8000;
	id_ = id;
	sendIcmpPingHdr->id_ = htons(id);
	sendIcmpPingHdr->seq_ = 0;

	memcpy(sendIcmpData, icmpData, icmpDataLen);

	icmpPingHdr->data();

	sendPacket_.clear();
	GBuf buf(pbyte(sendPacketByteArray_.data()), sendPacketByteArray_.size());
	sendPacket_.buf_ = buf;
	sendPacket_.ipHdr_ = sendIpHdr;
	sendPacket_.icmpHdr_ = sendIcmpPingHdr;

	hostHopIp_ = icmpKey.dip_;

	{
		QMutexLocker ml(&tr->icmpMgr_);
		tr->icmpMgr_.insert(icmpKey, this);
	}
}

GTraceRoute::IcmpThread::~IcmpThread() {
	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);
	{
		QMutexLocker ml(&tr->icmpMgr_);
		tr->icmpMgr_.remove(icmpKey_);
	}
}

void GTraceRoute::IcmpThread::run() {
	qDebug() << "beg";
	GTraceRoute* tr = PTraceRoute(parent());
	Q_ASSERT(tr != nullptr);

	if (swe_.wait(tr->startTimeout_)) return;
	GIpHdr* ipHdr = sendPacket_.ipHdr_;
	Q_ASSERT(ipHdr != nullptr);
	GIcmpPingHdr* icmpPingHdr = PIcmpPingHdr(sendPacket_.icmpHdr_);
	Q_ASSERT(icmpPingHdr != nullptr);
	for (uint8_t ttl = 1; ttl <= tr->maxHop_; ttl++) {
		ipHdr->tos_ = 0x44;
		ipHdr->ttl_ = ttl;
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));

		icmpPingHdr->seq_ = htons(ttl);
		icmpPingHdr->sum_ = htons(GIcmpHdr::calcChecksum(ipHdr, icmpPingHdr));

		for (int q = 0; q < tr->queryCount_; q++) {
			tr->rawIpSocketWrite_->write(&sendPacket_);
			swe_.wait(tr->sendTimeout_);
			if (!tr->active()) break;
		}

		if (!tr->active()) break;
		swe_.wait(tr->ttlChangeTimeout_);
	}
	if (tr->active()) swe_.wait(tr->stopTimeout_);

	writeLog();
	qDebug() << "end";
}

bool GTraceRoute::IcmpThread::processHostResponse(GIcmpPingHdr* icmpPingHdr) {
	uint16_t id = icmpPingHdr->id();
	if (id != id_) return false;
	int hopNo = icmpPingHdr->seq();
	if (hostHopNo_ != 0 && hopNo >= hostHopNo_) return false;
	hostHopNo_ = hopNo;
	GIp hopIp = icmpKey_.dip_;
	qDebug() << QString("%1 %2").arg(hopNo).arg(QString(hopIp));
	return true;
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GTraceRoute::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		return new GPropItemInterface(param);
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
