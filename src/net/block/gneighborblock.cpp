#include "gneighborblock.h"

// ----------------------------------------------------------------------------
// GNeighborBlock
// ----------------------------------------------------------------------------
GNeighborBlock::GNeighborBlock(QObject* parent) : GStateObj(parent) {
#ifndef Q_OS_ANDROID
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));
	if (entry != nullptr) {
		GIntf* intf = entry->intf();
		if (intf != nullptr)
			intfName_ = intf->name();
	}
#else
	intfName_ = "wlan0";
#endif // Q_OS_ANDROID
}

GNeighborBlock::~GNeighborBlock() {
	close();
}

bool GNeighborBlock::doOpen() {
	if (!enabled_) return true;

	intf_ = GNetInfo::instance().intfList().findByName(intfName_);
	if (intf_ == nullptr) {
		QString msg = QString("can not find interface for %1").arg(intfName_);
		SET_ERR(GErr::ValueIsNull, msg);
		return false;
	}

	return true;
}

bool GNeighborBlock::doClose() {
	if (!enabled_) return true;

	intf_ = nullptr;

	return true;
}

void GNeighborBlock::block(GPacket* packet) {
	if (!enabled_) return;
	GIpHdr* ipHdr = packet->ipHdr_;
	if  (ipHdr == nullptr) return;
	GIp sip = ipHdr->sip();
	GIp dip = ipHdr->dip();

	bool block = false;
	GIp myIp = intf_->ip();
	if (sip == myIp && dip != myIp) {
		if (dip != intf_->gateway() && intf_->isSameLanIp(dip))
			block = true;
	}
	if (sip != myIp && dip == myIp) {
		if (sip != intf_->gateway() && intf_->isSameLanIp(sip))
			block = true;
	}

	if (block) {
		packet->ctrl_.block_ = true;
		emit blocked(packet);
	}
}

void GNeighborBlock::unblock(GPacket* packet) {
	if (!enabled_) return;
	packet->ctrl_.block_ = false;
	emit unblocked(packet);
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-interface.h"
GPropItem* GNeighborBlock::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "intfName") {
		return new GPropItemInterface(param);
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB
