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
#if defined(Q_OS_ANDROID) || defined(Q_OS_ANDROID_GILGIL)
#include "net/demon/gdemonclient.h"

GRtm::GRtm() {
	GDemonClient& client = GDemonClient::instance("127.0.0.1", GDemon::DefaultPort);
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
#endif

#if defined(Q_OS_LINUX) && (!defined(Q_OS_ANDROID) || !defined(Q_OS_ANDROID_GILGIL))
//
// ip route show table 0 output
//
// [kali linux]
// default via 10.2.2.1 dev eth0 proto dhcp metric 100 (A)
// 10.2.2.0/24 dev eth0 proto kernel scope link src 10.2.2.3 metric 100 (B)
//
// [android]
// default via 10.2.2.1 dev wlan0  table 1021  proto static (C)
// 10.2.2.0/24 dev wlan0  proto kernel  scope link  src 10.2.2.189 (D)
//
GRtm::GRtm() {
	std::string command("ip route show table 0");
	FILE* p = popen(command.data(), "r");
	if (p == nullptr) {
		qFatal("failed to call %s", command.data());
		return;
	}

	while (true) {
		char buf[256];
		if (std::fgets(buf, 256, p) == nullptr) break;
		GRtmEntry entry;
		if (checkA(buf, &entry))
			push_back(entry);
		else if (checkB(buf, &entry))
			push_back(entry);
		else if (checkC(buf, &entry))
			push_back(entry);
		else if (checkD(buf, &entry))
			push_back(entry);
	}
	pclose(p);
}
#endif

#ifdef Q_OS_WIN
#include "_win/gipadapterinfo.h"
#include "_win/gipforwardtable.h"

GRtm::GRtm() {
	PMIB_IPFORWARDTABLE table = GIpForwardTable::instance().ipForwardTable_;
	for (int i = 0; i < int(table->dwNumEntries); i++) {
		PMIB_IPFORWARDROW row = &table->table[i];
		GRtmEntry entry;
		IF_INDEX ifIndex = row->dwForwardIfIndex;
		PIP_ADAPTER_INFO adapter = GIpAdapterInfo::instance().findByComboIndex(ifIndex);
		if (adapter == nullptr) continue;
		QString adapterName = adapter->AdapterName;
		Q_ASSERT(adapterName != "");
		entry.intfName_ = adapterName;
		entry.dst_ = ntohl(row->dwForwardDest);
		entry.gateway_ = ntohl(row->dwForwardNextHop);
		entry.mask_ = ntohl(row->dwForwardMask);
		entry.metric_ = int(row->dwForwardMetric1);
		// intf_ is initialized in GNetInfo
		append(entry);
	}
}
#endif

GRtm::~GRtm() {
	clear();
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

#if defined(Q_OS_LINUX) && (!defined(Q_OS_ANDROID) || !defined(Q_OS_ANDROID_GILGIL))
bool GRtm::checkA(char* buf, GRtmEntry* entry) {
	char gateway[256];
	char intf[256];
	int metric;
	// default via 10.2.2.1 dev eth0 proto dhcp metric 100 (A)
	int res = sscanf(buf, "default via %s dev %s proto dhcp metric %d", gateway, intf, &metric);
	if (res == 3) {
		entry->gateway_ = GIp(gateway);
		entry->intfName_ = intf;
		entry->metric_ = metric;
		return true;
	}
	return false;
}

bool GRtm::checkB(char* buf, GRtmEntry* entry) {
	char cidr[256];
	char intf[256];
	char myip[256];
	int metric;
	// 10.2.2.0/24 dev eth0 proto kernel scope link src 10.2.2.3 metric 100 (B)
	int res  = sscanf(buf, "%s dev %s proto kernel scope link src %s metric %d", cidr, intf, myip, &metric);
	if (res == 4) {
		uint32_t dst;
		uint32_t mask;
		if (!decodeCidr(cidr, &dst, &mask)) return false;
		entry->dst_ = dst;
		entry->mask_ = mask;
		entry->intfName_ = intf;
		entry->metric_ = metric;
		return true;
	}
	return false;
}

bool GRtm::checkC(char* buf, GRtmEntry* entry) {
	char gateway[256];
	char intf[256];
	// default via 10.2.2.1 dev wlan0  table 1021  proto static (C)
	int res = sscanf(buf, "default via %s dev %s", gateway, intf);
	if (res == 2) {
		entry->gateway_ = GIp(gateway);
		entry->intfName_ = intf;
		return true;
	}
	return false;
}

bool GRtm::checkD(char* buf, GRtmEntry* entry) {
	char cidr[256];
	char intf[256];
	char ip[256];
	// 10.2.2.0/24 dev wlan0  proto kernel  scope link  src 10.2.2.189 (D)
	int res = sscanf(buf, "%s dev %s proto kernel scope link src %s", cidr, intf, ip);
	if (res == 3) {
		uint32_t dst;
		uint32_t mask;
		if (!decodeCidr(cidr, &dst, &mask)) return false;
		entry->dst_ = dst;
		entry->mask_ = mask;
		entry->intfName_ = intf;
		return true;
	}
	return false;
}

bool GRtm::decodeCidr(std::string cidr, uint32_t* dst, uint32_t* mask) {
	size_t found = cidr.find("/");
	if (found == std::string::npos) return false;
	std::string dstStr = cidr.substr(0, found);
	*dst = GIp(dstStr.data());
	std::string maskStr = cidr.substr(found + 1);
	*mask = numberToMask(std::stoi(maskStr.data()));
	return true;
}

uint32_t GRtm::numberToMask(int number) {
	uint32_t res = 0;
	for (int i = 0; i < number; i++) {
		res = (res >> 1);
		res |= 0x80000000;
	}
	return res;
}
#endif

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
