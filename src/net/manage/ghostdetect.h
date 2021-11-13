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

#include "net/gatm.h"

// ----------------------------------------------------------------------------
// GHostDetect
// ----------------------------------------------------------------------------
struct G_EXPORT GHostDetect : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(bool checkDhcp MEMBER checkDhcp_)
	Q_PROPERTY(bool checkArp MEMBER checkArp_)
	Q_PROPERTY(bool checkIp MEMBER checkIp_)
	Q_PROPERTY(QString intfName MEMBER intfName_)

public:
	bool enabled_{true};
	bool checkDhcp_{true};
	bool checkArp_{false};
	bool checkIp_{false};
	QString intfName_;

public:
	Q_INVOKABLE GHostDetect(QObject* parent = nullptr);
	~GHostDetect() override;

protected:
	bool doOpen() override;
	bool doClose() override;

	GIntf* intf_{nullptr};
	GAtm atm_;
	GIp myIp_{0};
	GMac myMac_{GMac::nullMac()};
	GIp gwIp_{0};
	QElapsedTimer et_;

protected:
	bool processDhcp(GPacket* packet, GMac* mac, GIp* ip, QString* hostName);
	bool processArp(GEthHdr* ethHdr, GArpHdr* arpHdr, GMac* mac, GIp* ip);
	bool processIp(GEthHdr* ethHdr, GIpHdr* ipHdr, GMac* mac, GIp* ip);

public slots:
	void detect(GPacket* packet);

public:
	struct Host {
		Host() {};
		Host(GMac mac, GIp ip) : mac_(mac), ip_(ip) {}
		Host(GMac mac, GIp ip, QString hostName): mac_(mac), ip_(ip), hostName_(hostName) {}

		GMac mac_{GMac::nullMac()};
		GIp ip_{0};
		QString hostName_;
		QString nickName_;
		qint64 lastAccess_{0};

		QString defaultName() {
			if(!nickName_.isNull())
				return nickName_;
			if(!hostName_.isNull())
				return hostName_;
			return QString(ip_);
		}
	};

	struct HostMap : QHash<GMac, Host> {
		QMutex m_;
	} hosts_;

signals:
	void hostDetected(GHostDetect::Host* host);
	void hostDeleted(GHostDetect::Host* host);
};
