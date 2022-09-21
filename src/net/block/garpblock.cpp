#include "garpblock.h"

GArpBlock::GArpBlock(QObject* parent) : GStateObj(parent) {
}

GArpBlock::~GArpBlock() {
	close();
}

bool GArpBlock::doOpen() {
	if (!enabled_) return true;

	if (pcapDevice_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "pcapDevice is null");
		return false;
	}

	if (hostMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "hostMgr is null");
		return false;
	}

	itemOffset_ = hostMgr_->requestItems_.request(this, sizeof(Item));
	hostMgr_->managables_.insert(this);

	GIntf* intf = pcapDevice_->intf();
	Q_ASSERT(intf != nullptr);
	// infectPacket_.ethHdr_.dmac_ // set later
	infectPacket_.ethHdr_.smac_ = intf->mac();
	infectPacket_.ethHdr_.type_ = htons(GEthHdr::Arp);

	infectPacket_.arpHdr_.hrd_ = htons(GArpHdr::ETHER);
	infectPacket_.arpHdr_.pro_ = htons(GEthHdr::Ip4);
	infectPacket_.arpHdr_.hln_ = GMac::Size;
	infectPacket_.arpHdr_.pln_ = GIp::Size;
	infectPacket_.arpHdr_.op_ = htons(GArpHdr::Reply);
	infectPacket_.arpHdr_.smac_ = intf->mac();
	infectPacket_.arpHdr_.sip_ = htonl(intf->ip());
	// infectPacket_.arpHdr_.tmac_ // set later
	// infectPacket_.arpHdr_.tip_ // set later

	infectThread_.start();

	return true;
}

bool GArpBlock::doClose() {
	if (!enabled_) return true;

	infectThread_.we_.wakeAll();
	infectThread_.quit();
	infectThread_.wait();

	return true;
}

void GArpBlock::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	Item* item = PItem(hostValue->mem(itemOffset_));
	new (item) Item(defaultPolicy_, mac, hostValue->ip_);
}

void GArpBlock::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)mac;
	Item* item = PItem(hostValue->mem(itemOffset_));
	item->~Item();
}

void GArpBlock::InfectThread::run() {
	qDebug() << ""; // gilgil temp 2022.04.08
}

void GArpBlock::block(GPacket* packet) {
	qDebug() << ""; // gilgil temp 2022.04.08
	if (packet->arpHdr_ == nullptr) return;
}
