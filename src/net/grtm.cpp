#include <QDebug>
#include "grtm.h"

// ----------------------------------------------------------------------------
// GRtmEntry
// ----------------------------------------------------------------------------
bool GRtmEntry::operator==(const GRtmEntry& r) const {
	if (dst_ != r.dst_) return false;
	if (mask_ != r.mask_) return false;
	if (gateway_ != r.gateway_) return false;
	if (metric_ != r.metric_) return false;
	if (intf_ != r.intf_) return false;
	return true;
};

// ----- gilgil temp 2021.03.19 -----
/*
uint qHash(GRtmEntry q) {
	return uint(q.dst() + q.mask() + q.gateway() + uint32_t(q.metric()));
}
*/
// ----------------------------------

// ----------------------------------------------------------------------------
// GRtm
// ----------------------------------------------------------------------------
GRtm::GRtm() {
}

GRtm::~GRtm() {
	clear();
}

GIp GRtm::findGateway(QString intfName, GIp ip) {
	for (GRtmEntry& entry: *this) {
		if (entry.intf() == nullptr) continue;
		if (entry.intf()->name() != intfName) continue;
		if (entry.gateway_ == 0) continue;
		if (entry.gateway_ == ip) continue;
		return entry.gateway_;
	}
	return GIp(0);
}

GRtmEntry* GRtm::getBestEntry(GIp ip) {
	GRtmEntry* res = nullptr;

	int _count = count();
	for (int i = 0; i < _count; i++) {
		GRtmEntry& entry = const_cast<GRtmEntry&>(at(i));

		if ((entry.dst_ & entry.mask_) != (ip & entry.mask_)) continue;
		if (res == nullptr) {
			res = &entry;
			continue;
		}
		if (entry.mask_ > res->mask_) {
			res = &entry;
			continue;
		} else
		if (entry.mask_ == res->mask_) {
			if (entry.metric_ < res->metric_) {
				res = &entry;
				continue;
			}
		}
	}

	return res;
}

// ----------------------------------------------------------------------------
// GRemoteRtm
// ----------------------------------------------------------------------------
#include "net/demon/gdemonclient.h"

GRemoteRtm::GRemoteRtm(QString ip, quint16 port) {
	GDemonClient& client = GDemonClient::instance(ip.toStdString(), port);
	GDemon::GetRtmRes res = client.getRtm();
	for (GDemon::RtmEntry& entry: res.rtm_) {
		GRtmEntry rtmEntry;
		rtmEntry.dst_ = entry.dst_;
		rtmEntry.mask_ = entry.mask_;
		rtmEntry.gateway_ = entry.gateway_;
		rtmEntry.metric_ = entry.metric_;
		rtmEntry.intfName_ = entry.intfName_.data();
		push_back(rtmEntry);
	}
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>
#include "net/gnetinfo.h"

TEST(GRtm, loadTest) {
	GRtm& rtm = GNetInfo::instance().rtm();
	qDebug() << "Routing Table Manager : count =" << rtm.count();
	for (GRtm::iterator it = rtm.begin(); it != rtm.end(); it++) {
		GRtmEntry& entry = *it;
		qDebug() << QString("dst=%1 mask=%2 gateway=%3 intf=%4 metric=%5").arg(
			QString(entry.dst()),
			QString(entry.mask()),
			QString(entry.gateway()),
			entry.intf()->name(),
			QString::number(entry.metric())
			);
	}
}

TEST(GRtm, bestTest) {
	GRtm& rtm = GNetInfo::instance().rtm();
	GRtmEntry* entry = rtm.getBestEntry(QString("8.8.8.8"));
	EXPECT_NE(entry, nullptr);
	qDebug() << "best entry for 8.8.8.8 is" << entry->intf()->name();
}

#endif // GTEST
