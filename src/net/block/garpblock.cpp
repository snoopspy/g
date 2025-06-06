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

	if (hostDb_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "hostDb is null");
		return false;
	}

	write_.intfName_ = pcapDevice_->intfName_;
	if (!write_.open()) {
		err = write_.err;
		return false;
	}

	GIntf* intf = write_.intf();
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

	if (atm_.find(gwIp_) == atm_.end()) {
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
	}
	gwMac_ = atm_.find(gwIp_).value();
	atm_.close();

	// sendPacket_.ethHdr_.dmac_ // set later
	sendPacket_.ethHdr_.smac_ = mac_;
	sendPacket_.ethHdr_.type_ = htons(GEthHdr::Arp);

	sendPacket_.arpHdr_.hrd_ = htons(GArpHdr::Ether);
	sendPacket_.arpHdr_.pro_ = htons(GEthHdr::Ip4);
	sendPacket_.arpHdr_.hln_ = GMac::Size;
	sendPacket_.arpHdr_.pln_ = GIp::Size;
	// sendPacket_.arpHdr_.op_ // set later
	// sendPacket_.arpHdr_.smac_ // set later
	// sendPacket_.arpHdr_.sip_ = // set later
	// sendPacket_.arpHdr_.tmac_ // set later
	// sendPacket_.arpHdr_.tip_ // set later

	itemMap_.clear();
	infectThread_.start();

	return true;
}

bool GArpBlock::doClose() {
	if (!enabled_) return true;

	infectThread_.swe_.wakeAll();
	infectThread_.quit();
	infectThread_.wait();

	write_.close();

	return true;
}

void GArpBlock::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	Item* item = getItem(hostValue);
	new (item) Item;
	item->mac_ = mac;
	item->ip_ = hostValue->ip_;
	Policy policy = defaultPolicy_;
	GHostDb::Item dbItem;
	if (!hostDb_->selectHost(mac, &dbItem)) {
		qWarning() << QString("selectHost(%1) return false").arg(QString(mac_));
	}
	switch (dbItem.mode_) {
		case GHostDb::Auto :
			break;
		case GHostDb::Allow :
			policy = Allow;
			break;
		case GHostDb::Block :
			policy = Block;
			break;
	}
	item->policy_ = policy;
	if (item->policy_ == Block)
		infect(item, GArpHdr::Request);

	{
		QMutexLocker ml(&itemMap_);
		itemMap_.insert(mac, item);
	}
}

void GArpBlock::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	Item* item = getItem(hostValue);
	Q_ASSERT(item->mac_ == mac);
	if (item->policy_ == Block)
		recover(item, GArpHdr::Request);
	item->~Item();

	{
		QMutexLocker ml(&itemMap_);
		itemMap_.remove(mac);
	}
}

void GArpBlock::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)mac;
	Item* item = getItem(hostValue);
	item->ip_ = hostValue->ip_;
}

void GArpBlock::infect(Item* item, uint16_t operation) {
	QMutexLocker ml(&sendMutex_);

	if (attackDirection_ == Host || attackDirection_ == Both) { // To Host
		sendPacket_.ethHdr_.dmac_ = item->mac_;
		sendPacket_.arpHdr_.op_ = htons(operation);
		sendPacket_.arpHdr_.smac_ = fakeMac_;
		sendPacket_.arpHdr_.sip_ = htonl(gwIp_);
		sendPacket_.arpHdr_.tmac_ = item->mac_;
		sendPacket_.arpHdr_.tip_ = htonl(item->ip_);

		write_.writeBuf(GBuf(pbyte(&sendPacket_), sizeof(GEthArpPacket)));
	}

	if (attackDirection_ == Gateway || attackDirection_ == Both) { // To Gateway
		sendPacket_.ethHdr_.dmac_ = gwMac_;
		sendPacket_.arpHdr_.op_ = htons(operation);
		sendPacket_.arpHdr_.smac_ = fakeMac_;
		sendPacket_.arpHdr_.sip_ = htonl(item->ip_);
		sendPacket_.arpHdr_.tmac_ = gwMac_;
		sendPacket_.arpHdr_.tip_ = htonl(gwIp_);

		write_.writeBuf(GBuf(pbyte(&sendPacket_), sizeof(GEthArpPacket)));
	}
}

void GArpBlock::recover(Item *item, uint16_t operation)
{
	QMutexLocker ml(&sendMutex_);

	if (attackDirection_ == Host || attackDirection_ == Both) { // To Host
		sendPacket_.ethHdr_.dmac_ = item->mac_;
		sendPacket_.arpHdr_.op_ = htons(operation);
		sendPacket_.arpHdr_.smac_ = gwMac_;
		sendPacket_.arpHdr_.sip_ = htonl(gwIp_);
		sendPacket_.arpHdr_.tmac_ = item->mac_;
		sendPacket_.arpHdr_.tip_ = htonl(item->ip_);

		write_.writeBuf(GBuf(pbyte(&sendPacket_), sizeof(GEthArpPacket)));
	}

	if (attackDirection_ == Gateway || attackDirection_ == Both) { // To Gateway
		sendPacket_.ethHdr_.dmac_ = gwMac_;
		sendPacket_.arpHdr_.op_ = htons(operation);
		sendPacket_.arpHdr_.smac_ = item->mac_;
		sendPacket_.arpHdr_.sip_ = htonl(item->ip_);
		sendPacket_.arpHdr_.tmac_ = gwMac_;
		sendPacket_.arpHdr_.tip_ = htonl(gwIp_);

		write_.writeBuf(GBuf(pbyte(&sendPacket_), sizeof(GEthArpPacket)));
	}
}

void GArpBlock::InfectThread::run() {
	ItemMap* itemMap = &arpBlock_->itemMap_;

	while (arpBlock_->active()) {
		{
			QMutexLocker ml(itemMap);
			for (Item* item: *itemMap) {
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
		QMutexLocker ml(itemMap);
		for (Item* item: *itemMap) {
			if (item->policy_ == Block)
				arpBlock_->recover(item, GArpHdr::Request);
		}
	}
}
