#pragma once

#include <GGraph>
#include <GAutoArpSpoof>
#include <GFind>
#include <GTcpBlock>
#include <GUdpBlock>
#include <GDnsBlock>
#include <GTcpFlowMgr>
#include <GCookieHijack>
#include <GBpFilter>
#include <GBlock>
#include <GCommand>
#include "webserver.h"

struct G_EXPORT CookieHijack : GGraph {
	Q_OBJECT
	Q_PROPERTY(QStringList httpSiteList MEMBER httpSiteList_)
	Q_PROPERTY(QStringList httpsSiteList MEMBER httpsSiteList_)
	Q_PROPERTY(QString prefix MEMBER prefix_)
	Q_PROPERTY(QString firefoxDir MEMBER firefoxDir_)
	Q_PROPERTY(bool stealCookie MEMBER stealCookie_)

	Q_PROPERTY(GObjRef webServer READ getWebServer)
	Q_PROPERTY(GObjRef autoArpSpoof READ getGAutoArpSpoof)
	Q_PROPERTY(GObjRef find READ getFind)
	Q_PROPERTY(GObjRef tcpBlock READ getTcpBlock)
	Q_PROPERTY(GObjRef dnsBlock READ getDnsBlock)
	Q_PROPERTY(GObjRef dnsBlockDnsServer READ getDnsBlockDnsServer)
	Q_PROPERTY(GObjRef tcpFlowMgr READ getTcpFlowMgr)
	Q_PROPERTY(GObjRef cookieHijack READ getCookieHijack)
	Q_PROPERTY(GObjRef bpFilter READ getBpFilter)
	Q_PROPERTY(GObjRef blockExternal READ getBlockExternal)
	Q_PROPERTY(GObjRef tcpBlockExternal READ getTcpBlockExternal)
	Q_PROPERTY(GObjRef udpBlockExternal READ getUdpBlockExternal)
	Q_PROPERTY(GObjRef command READ getCommand)

public:
	GObjRef getWebServer() { return &webServer_; }
	GObjRef getGAutoArpSpoof() { return &autoArpSpoof_; }
	GObjRef getFind() { return &find_; }
	GObjRef getTcpBlock() { return &tcpBlock_; }
	GObjRef getDnsBlock() { return &dnsBlock_; }
	GObjRef getDnsBlockDnsServer() { return &dnsBlockDnsServer_; }
	GObjRef getTcpFlowMgr() { return &tcpFlowMgr_; }
	GObjRef getCookieHijack() { return &cookieHijack_; }
	GObjRef getBpFilter() { return &bpFilter_; }
	GObjRef getBlockExternal() { return &blockExternal_; }
	GObjRef getTcpBlockExternal() { return &tcpBlockExternal_; }
	GObjRef getUdpBlockExternal() { return &udpBlockExternal_; }
	GObjRef getCommand() { return &command_; }

public:
	QStringList httpSiteList_;
	QStringList httpsSiteList_;
	QString prefix_;
	QString firefoxDir_;
	bool stealCookie_{false};

	WebServer webServer_{this};
	GAutoArpSpoof autoArpSpoof_{this};
	GFind find_{this};
	GTcpBlock tcpBlock_{this};
	GDnsBlock dnsBlock_{this};
	GDnsBlock dnsBlockDnsServer_{this};
	GTcpFlowMgr tcpFlowMgr_{this};
	GCookieHijack cookieHijack_{this};
	GBpFilter bpFilter_{this};
	GBlock blockExternal_{this};
	GTcpBlock tcpBlockExternal_{this};
	GUdpBlock udpBlockExternal_{this};
	GCommand command_{this};

public:
	Q_INVOKABLE CookieHijack(QObject* parent = nullptr);
	~CookieHijack() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	struct Site {
		QString name;
		bool ssl;
	};
	QList<Site> totalSiteList_;
	QStringList getHttpResponse(int siteNo, QString cookie);

public slots:
	void hijack(GPacket* packet);

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
