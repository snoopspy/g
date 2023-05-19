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

    mac_ = intf->mac();
    ip_ = intf->ip();
    if (ip_ == 0) {
		SET_ERR(GErr::ValueIsZero, "ip is zero");
		return false;
	}
    gwIp_ = intf->gateway();
    if (gwIp_ == 0) {
		SET_ERR(GErr::ValueIsZero, "gateway is zero");
		return false;
	}

	atm_.intfName_ = pcapDevice_->intfName_;
	if (!atm_.open()) {
		err = atm_.err;
		return false;
	}
    atm_.insert(gwIp_, GMac::nullMac());
	if (!atm_.wait()) {
		err = atm_.err;
		return false;
	}
    gwMac_ = atm_.find(gwIp_).value();
	atm_.close();

    // arpPacket_.ethHdr_.dmac_ // set later
    arpPacket_.ethHdr_.smac_ = mac_;
    arpPacket_.ethHdr_.type_ = htons(GEthHdr::Arp);

    arpPacket_.arpHdr_.hrd_ = htons(GArpHdr::ETHER);
    arpPacket_.arpHdr_.pro_ = htons(GEthHdr::Ip4);
    arpPacket_.arpHdr_.hln_ = GMac::Size;
    arpPacket_.arpHdr_.pln_ = GIp::Size;
    // arpPacket_.arpHdr_.op_ // set later
    // arpPacket_.arpHdr_.smac_ // set later
    // arpPacket_.arpHdr_.sip_ = // set later
    // arpPacket_.arpHdr_.tmac_ // set later
    // arpPacket_.arpHdr_.tip_ // set later

	itemList_.clear();

	infectThread_.start();

	return true;
}

bool GArpBlock::doClose() {
	if (!enabled_) return true;

	infectThread_.swe_.wakeAll();
	infectThread_.quit();
	infectThread_.wait();

	return true;
}

void GArpBlock::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
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
	(void)mac;

	Item* item = PItem(hostValue->mem(itemOffset_));
	if (item->policy_ == Block)
		recover(item, GArpHdr::Request);
	item->~Item();

	{
		QMutexLocker ml(&itemList_.m_);
		itemList_.removeOne(item);
	}
}

void GArpBlock::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)mac;

	Item* item = PItem(hostValue->mem(itemOffset_));
	item->ip_ = hostValue->ip_;
}

void GArpBlock::infect(Item* item, uint16_t operation) {
    if (attackDirection_ == Host || attackDirection_ == Both) { // To Host
        arpPacket_.ethHdr_.dmac_ = item->mac_;
        arpPacket_.arpHdr_.op_ = htons(operation);
        arpPacket_.arpHdr_.smac_ = fakeMac_;
        arpPacket_.arpHdr_.sip_ = htonl(gwIp_);
        arpPacket_.arpHdr_.tmac_ = item->mac_;
        arpPacket_.arpHdr_.tip_ = htonl(item->ip_);

        if (pcapDevice_->active())
            pcapDevice_->write(GBuf(pbyte(&arpPacket_),sizeof(GEthArpPacket)));
    }

    if (attackDirection_ == Gateway || attackDirection_ == Both) { // To Gateway
        arpPacket_.ethHdr_.dmac_ = gwMac_;

        arpPacket_.arpHdr_.op_ = htons(operation);
        arpPacket_.arpHdr_.smac_ = fakeMac_;
        arpPacket_.arpHdr_.sip_ = htonl(item->ip_);
        arpPacket_.arpHdr_.tmac_ = gwMac_;
        arpPacket_.arpHdr_.tip_ = htonl(gwIp_);

        if (pcapDevice_->active())
            pcapDevice_->write(GBuf(pbyte(&arpPacket_),sizeof(GEthArpPacket)));
    }
}

void GArpBlock::recover(Item* item, uint16_t operation) {
	qDebug() << QString(item->ip_) << QString(item->mac_); // gilgil temp 2022.10.15

    if (attackDirection_ == Host || attackDirection_ == Both) { // To Host
        arpPacket_.ethHdr_.dmac_ = item->mac_;
        arpPacket_.arpHdr_.op_ = htons(operation);
        arpPacket_.arpHdr_.smac_ = gwMac_;
        arpPacket_.arpHdr_.sip_ = htonl(gwIp_);
        arpPacket_.arpHdr_.tmac_ = item->mac_;
        arpPacket_.arpHdr_.tip_ = htonl(item->ip_);

        if (pcapDevice_->active())
            pcapDevice_->write(GBuf(pbyte(&arpPacket_),sizeof(GEthArpPacket)));
    }

    if (attackDirection_ == Gateway || attackDirection_ == Both) { // To Gateway
        arpPacket_.ethHdr_.dmac_ = gwMac_;
        arpPacket_.arpHdr_.op_ = htons(operation);
        arpPacket_.arpHdr_.smac_ = item->mac_;
        arpPacket_.arpHdr_.sip_ = htonl(item->ip_);
        arpPacket_.arpHdr_.tmac_ = gwMac_;
        arpPacket_.arpHdr_.tip_ = htonl(gwIp_);

        if (pcapDevice_->active())
            pcapDevice_->write(GBuf(pbyte(&arpPacket_),sizeof(GEthArpPacket)));
    }
}

void GArpBlock::InfectThread::run() {
	ItemList* itemList = &arpBlock_->itemList_;

	while (arpBlock_->active()) {
		{
			QMutexLocker ml(&itemList->m_);
			for (Item* item: *itemList) {
				if (item->policy_ != Block) continue;
				arpBlock_->infect(item, GArpHdr::Reply);
				if (swe_.wait(arpBlock_->sendInterval_)) break;
				if (!arpBlock_->active()) break;
			}
		}
		if (!arpBlock_->active()) break;
		if (swe_.wait(arpBlock_->infectInterval_)) break;
	}

	{
		QMutexLocker ml(&itemList->m_);
		for (Item* item: *itemList) {
			if (item->policy_ == Block)
				arpBlock_->recover(item, GArpHdr::Request);
		}
	}
}
