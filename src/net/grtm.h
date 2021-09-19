// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include <QList>
#include "gintf.h"

// ----------------------------------------------------------------------------
// GRtmEntry
// ----------------------------------------------------------------------------
struct G_EXPORT GRtmEntry {
	friend struct GRtm;
	friend struct GNetInfo;

public:
	GIp dst() const { return dst_; }
	GIp mask() const { return mask_; }
	GIp gateway() const { return gateway_; }
	int metric() const { return metric_; }
	GIntf* intf() const { return intf_; }

protected:
	GIp dst_{0};
	GIp mask_{0};
	GIp gateway_{0};
	int metric_{0};
	GIntf* intf_{nullptr};
	QString intfName_;

public:
	bool operator==(const GRtmEntry& r) const;
};

// ----------------------------------------------------------------------------
// GRtm(Routing Table Manager)
// ----------------------------------------------------------------------------
struct G_EXPORT GRtm : QList<GRtmEntry> {
	friend struct GNetInfo;
	friend struct GRtmEntry;

protected: // inherited singleton
	GRtm();
	virtual ~GRtm();

public:
	GRtmEntry* getBestEntry(GIp ip);
	GIp findGateway(QString intfName, GIp ip);

#if defined(Q_OS_LINUX) && (!defined(Q_OS_ANDROID) || !defined(Q_OS_ANDROID_GILGIL))
protected:
	static bool checkA(char* buf, GRtmEntry* entry);
	static bool checkB(char* buf, GRtmEntry* entry);
	static bool checkC(char* buf, GRtmEntry* entry);
	static bool checkD(char* buf, GRtmEntry* entry);
	static bool decodeCidr(std::string cidr, uint32_t* dst, uint32_t* mask);
	static uint32_t numberToMask(int number);
#endif
};
