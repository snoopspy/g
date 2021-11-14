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

#include <pcap.h>
#include <QList>
#include "gnet.h"
#include "gip.h"
#include "gmac.h"

// ----------------------------------------------------------------------------
// GIntf
// ----------------------------------------------------------------------------
struct G_EXPORT GIntf {
	friend struct GNetInfo;
	friend struct GIntfList;

public:
	int index() const { return index_; }
	QString name() const { return name_; }
	QString desc() const { return desc_; }
	GMac mac() const { return mac_; }
	GIp ip() const { return ip_; }
	GIp mask() const { return mask_; }
	GIp gateway() const { return gateway_; }

protected:
	int index_{-1};
	QString name_;
	QString desc_;
	GMac mac_{GMac::nullMac()};
	GIp ip_{0};
	GIp mask_{0};
	GIp gateway_{0};

protected:
	GIp ip_and_mask_{0}; // used for isSameLanIP

public:
	GIntf() {}

public:
	bool isSameLanIp(GIp ip) { return (ip_and_mask_) == (ip & mask_); }
	GIp getAdjIp(GIp ip) { return isSameLanIp(ip) ? ip : gateway_; }
	GIp getBeginIp() { return (ip_ & mask_) + 1; }
	GIp getEndIp() { return (ip_ | ~mask_);}

public:
	bool operator==(const GIntf& r) const;
};

// ----------------------------------------------------------------------------
// GIntfList
// ----------------------------------------------------------------------------
struct G_EXPORT GIntfList : QList<GIntf> {
	friend struct GNetInfo;

protected: // singleton
	GIntfList();
	virtual ~GIntfList() {}

public:
	GIntf* findByName(QString name);
	GIntf* findByIp(GIp ip);
};
