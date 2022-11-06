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
	if (intf == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "intf is null");
		return false;
	}
	GIp ip = intf->ip();
	if (ip == 0) {
		SET_ERR(GErr::ValueIsZero, "ip is zero");
		return false;
	}
	GIp gateway = intf->gateway();
	if (gateway == 0) {
		SET_ERR(GErr::ValueIsZero, "gateway is zero");
		return false;
	}

	atm_.intfName_ = pcapDevice_->intfName_;
	if (!atm_.open()) {
		err = atm_.err;
		return false;
	}
	atm_.insert(gateway, GMac::nullMac());
	if (!atm_.wait()) {
		err = atm_.err;
		return false;
	}
	GMac gatewayMac = atm_.find(gateway).value();
	atm_.close();

	// infectPacket_.ethHdr_.dmac_ // set later
	infectPacket_.ethHdr_.smac_ = intf->mac();
	infectPacket_.ethHdr_.type_ = htons(GEthHdr::Arp);

	infectPacket_.arpHdr_.hrd_ = htons(GArpHdr::ETHER);
	infectPacket_.arpHdr_.pro_ = htons(GEthHdr::Ip4);
	infectPacket_.arpHdr_.hln_ = GMac::Size;
	infectPacket_.arpHdr_.pln_ = GIp::Size;
	// infectPacket_.arpHdr_.op_ // set later
	infectPacket_.arpHdr_.smac_ = fakeMac_;
	infectPacket_.arpHdr_.sip_ = htonl(gateway);
	// infectPacket_.arpHdr_.tmac_ // set later
	// infectPacket_.arpHdr_.tip_ // set later

	recoverPacket_.arpHdr_.op_ = htons(GArpHdr::Request);
	recoverPacket_ = infectPacket_;
	recoverPacket_.arpHdr_.smac_ = gatewayMac;

	itemList_.clear();

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
	qDebug() << ""; // gilgil temp 2022.10.15
	Item* item = PItem(hostValue->mem(itemOffset_));
	new (item) Item(mac, hostValue->ip_, defaultPolicy_);
	if (item->policy_ == Block)
		infect(item, GArpHdr::Request);

	{
		QMutexLocker ml(&itemList_.m_);
		itemList_.push_back(item);
	}
}

void GArpBlock::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	qDebug() << ""; // gilgil temp 2022.10.15
	(void)mac;

	Item* item = PItem(hostValue->mem(itemOffset_));
	recover(item, GArpHdr::Request);
	item->~Item();
	{
		QMutexLocker ml(&itemList_.m_);
		itemList_.removeOne(item);
	}
}

void GArpBlock::infect(Item* item, uint16_t operation) {
	infectPacket_.ethHdr_.dmac_ = item->mac_;
	infectPacket_.arpHdr_.op_ = htons(operation);
	infectPacket_.arpHdr_.tmac_ = item->mac_;
	infectPacket_.arpHdr_.tip_ = htonl(item->ip_);
	if (pcapDevice_->active())
		pcapDevice_->write(GBuf(pbyte(&infectPacket_),sizeof(GEthArpPacket)));
}

void GArpBlock::recover(Item* item, uint16_t operation) {
	qDebug() << QString(item->ip_) << QString(item->mac_); // gilgil temp 2022.10.15
	recoverPacket_.ethHdr_.dmac_ = item->mac_;
	recoverPacket_.arpHdr_.op_ = htons(operation);
	recoverPacket_.arpHdr_.tmac_ = item->mac_;
	recoverPacket_.arpHdr_.tip_ = htonl(item->ip_);
	if (pcapDevice_->active())
		pcapDevice_->write(GBuf(pbyte(&recoverPacket_),sizeof(GEthArpPacket)));
}

void GArpBlock::InfectThread::run() {
	ItemList* itemList = &arpBlock_->itemList_;

	while (arpBlock_->active()) {
		{
			QMutexLocker ml(&itemList->m_);
			for (Item* item: *itemList) {
				if (item->policy_ != Block) continue;
				arpBlock_->infect(item, GArpHdr::Reply);
				if (we_.wait(arpBlock_->sendInterval_)) break;
				if (!arpBlock_->active()) break;
			}
		}
		if (!arpBlock_->active()) break;
		if (we_.wait(arpBlock_->infectInterval_)) break;
	}

	{
		QMutexLocker ml(&itemList->m_);
		for (Item* item: *itemList) {
			if (item->policy_ == Block)
				arpBlock_->recover(item, GArpHdr::Request);
		}
	}
}
