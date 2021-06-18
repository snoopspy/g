#include "gatm.h"
#include <QElapsedTimer>

// ----------------------------------------------------------------------------
// GAtm
// ----------------------------------------------------------------------------
template <typename BaseClass>
bool GAtm_<BaseClass>::allResolved() {
	for (GMac& mac: *this)
		if (mac.isNull()) return false;
	return true;
}

template <typename BaseClass>
void GAtm_<BaseClass>::deleteUnresolved() {
	for (GAtmMap::iterator it = begin(); it != end();) {
		GMac mac = it.value();
		if (mac.isNull()) {
			it = erase(it);
		} else {
			it++;
		}
	}
}

template <typename BaseClass>
bool GAtm_<BaseClass>::wait(GDuration timeout) {
	if (allResolved()) return true;

	if (!this->active()) {
		qWarning() << QString("not opened state %1").arg(this->metaObject()->className());
		return false;
	}

	GPacket::Dlt _dlt = this->dlt();
	if (_dlt != GPacket::Eth) {
		qWarning() << QString("invalid dlt %1").arg(GPacket::dltToString(_dlt));
		return false;
	}

	SendThread thread(this, timeout);
	thread.start();

	bool succeed = false;
	while (true) {
		if (allResolved()) {
			succeed = true;
			break;
		}

		if (thread.isFinished()) {
			QString msg = "can not resolve all ip";
			for (GAtmMap::iterator it = begin(); it != end(); it++) {
				GMac mac = it.value();
				if (mac.isNull()) {
					GIp ip = it.key();
					msg += " ";
					msg += QString(ip);
				}
			}
			qWarning() << msg;
			break;
		}

		GEthPacket packet;
		GPacket::Result res = this->read(&packet);
		if (res == GPacket::Eof) {
			qWarning() << "pcapDevice->read return GPacket::Eof";
			break;
		} else
		if (res == GPacket::Fail) {
			qWarning() << "pcapDevice->read return GPacket::Eof";
			break;
		} else
		if (res == GPacket::None) {
			continue;
		}

		GArpHdr* arpHdr = packet.arpHdr_;
		if (arpHdr == nullptr) continue;
		if (arpHdr->op() != GArpHdr::Reply) continue;

		GIp sip = arpHdr->sip();
		GMac smac = arpHdr->smac();
		GAtmMap::iterator it = find(sip);
		if (it != end()) {
			it.value() = smac;
			qDebug() << QString("resolved ip:%1 mac:%2").arg(QString(it.key()), QString(it.value()));
			if (allResolved()) {
				succeed = true;
				break;
			}
		}
	}
	thread.we_.wakeAll();
	thread.wait();
	return succeed;
}

template <typename BaseClass>
bool GAtm_<BaseClass>::sendQueries() {
	GEthArpHdr query;
	query.ethHdr_.dmac_ = GMac::broadcastMac();
	query.ethHdr_.smac_ = this->intf_->mac();
	query.ethHdr_.type_ = htons(GEthHdr::Arp);

	query.arpHdr_.hrd_ = htons(GArpHdr::ETHER);
	query.arpHdr_.pro_ = htons(GEthHdr::Ip4);
	query.arpHdr_.hln_ = sizeof(GMac);
	query.arpHdr_.pln_ = sizeof(GIp);
	query.arpHdr_.op_ = htons(GArpHdr::Request);
	query.arpHdr_.smac_ = this->intf_->mac();
	query.arpHdr_.sip_ = htonl(this->intf_->ip());
	query.arpHdr_.tmac_ = GMac::nullMac();
	GBuf queryBuf(pbyte(&query), sizeof(query));

	for (GAtmMap::iterator it = begin(); it != end(); it++) {
		GIp ip = it.key();
		GMac mac = it.value();
		if (mac.isNull()) {
			query.arpHdr_.tip_ = htonl(ip);
			GPacket::Result res = this->write(queryBuf);
			if (res != GPacket::Ok) {
				return false;
			}
		}
	}
	return true;
}

// --------------------------------------------------------------------------
// GAtm::SendThread
// --------------------------------------------------------------------------
template <typename BaseClass>
void GAtm_<BaseClass>::SendThread::run() {
	QElapsedTimer timer; timer.start();
	qint64 first = timer.elapsed();
	while (true) {
		if (!atm_->sendQueries())
			break;
		bool res = we_.wait(1000);
		if (res) break;
		qint64 now = timer.elapsed();
		if (now - first >= qint64(timeout_)) {
			qWarning() << "SendThread::run() timeout elapsed";
			break;
		}
	}

	GRemoteAtm* atm = dynamic_cast<GRemoteAtm*>(atm_);
	if (atm != nullptr) {
		atm->demonClient_->pcapClose(); // awaken read(&packet) in wait function by disconnect socket
	}
}

template struct GAtm_<GSyncPcapDevice>;
template struct GAtm_<GSyncRemotePcapDevice>;

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

#include "net/capture/gsyncpcapdevice.h"
TEST(GAtm, resolveTest) {
	GSyncPcapDevice device;

	ASSERT_TRUE(device.open());

	QString intfName = device.intfName_;
	ASSERT_NE(intfName, "");
	device.close();

	GInterface* intf = GNetInfo::instance().interfaceList().findByName(intfName);
	ASSERT_TRUE(intf != nullptr);

	GIp ip = intf->gateway();
	ASSERT_NE(ip, 0);

	GAtm atm;
	atm.intfName_ = intfName;
	EXPECT_TRUE(atm.open());
	atm.insert(ip, GMac::nullMac());
	EXPECT_TRUE(atm.wait());

	GMac mac = atm.find(ip).value();
	qDebug() << QString("ip:%1 mac:%2").arg(QString(ip), QString(mac));
}

#endif // GTEST
