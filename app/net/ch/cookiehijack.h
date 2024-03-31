#pragma once

#include <GGraph>
#include <GAutoArpSpoof>
#include <GFind>
#include <GTcpBlock>
#include <GDnsBlock>
#include <GTcpFlowMgr>
#include <GCookieHijack>
#include <GBpFilter>
#include <GBlock>
#include <GCommand>

struct G_EXPORT CookieHijack : GGraph {
	Q_OBJECT
	Q_PROPERTY(QString hackingSite MEMBER hackingSite_)
	Q_PROPERTY(QString prefix MEMBER prefix_)
	Q_PROPERTY(int httpPort MEMBER httpPort_)
	Q_PROPERTY(int httpsPort MEMBER httpsPort_)

	Q_PROPERTY(GObjRef autoArpSpoof READ getGAutoArpSpoof)
	Q_PROPERTY(GObjRef find READ getFind)
	Q_PROPERTY(GObjRef tcpBlock READ getTcpBlock)
	Q_PROPERTY(GObjRef dnsBlock READ getDnsBlock)
	Q_PROPERTY(GObjRef tcpFlowMgr READ getTcpFlowMgr)
	Q_PROPERTY(GObjRef cookieHijack READ getCookieHijack)
	Q_PROPERTY(GObjRef bpFilter READ getBpFilter)
	Q_PROPERTY(GObjRef block READ getBlock)
	Q_PROPERTY(GObjRef command READ getCommand)

public:
	GObjRef getGAutoArpSpoof() { return &autoArpSpoof_; }
	GObjRef getFind() { return &find_; }
	GObjRef getTcpBlock() { return &tcpBlock_; }
	GObjRef getDnsBlock() { return &dnsBlock_; }
	GObjRef getTcpFlowMgr() { return &tcpFlowMgr_; }
	GObjRef getCookieHijack() { return &cookieHijack_; }
	GObjRef getBpFilter() { return &bpFilter_; }
	GObjRef getBlock() { return &block_; }
	GObjRef getCommand() { return &command_; }

public:
	QString hackingSite_{"naver.com"};
	QString prefix_{"wifi"};
	int httpPort_{8080};
	int httpsPort_{4433};

	GAutoArpSpoof autoArpSpoof_{this};
	GFind find_{this};
	GTcpBlock tcpBlock_{this};
	GDnsBlock dnsBlock_{this};
	GTcpFlowMgr tcpFlowMgr_{this};
	GCookieHijack cookieHijack_{this};
	GBpFilter bpFilter_{this};
	GBlock block_{this};
	GCommand command_{this};


public:
	Q_INVOKABLE CookieHijack(QObject* parent = nullptr);
	~CookieHijack() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	const static int ColumnMac = 0;
	const static int ColumnType = 1;
	const static int ColumnChannel = 2;
	const static int ColumnSignal = 3;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
