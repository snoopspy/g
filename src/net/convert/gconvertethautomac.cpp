#include "gconvertethautomac.h"
#include <QMap>

// ----------------------------------------------------------------------------
// GConvertEthAutoMac
// ----------------------------------------------------------------------------
bool GConvertEthAutoMac::doOpen() {
	if (!GPcapDeviceWrite::doOpen())
		return false;

	if (dlt_ != GPacket::Eth) {
		QString msg = QString("data link layer(%1) must be GPacket::Eth").arg(GPacket::dltToString(dlt_));
		SET_ERR(GErr::FAIL, msg);
		return false;
	}

	Q_ASSERT(intf_ != nullptr);
	myMac_ = intf_->mac();
	myIp_ = intf_->ip();
	if (myMac_.isNull()) {
		QString msg = QString("not specified mac");
		SET_ERR(GErr::VALUE_IS_NOT_ZERO, msg);
		return false;
	}

	atm_.intfName_ = intfName_;
	if (!atm_.open()) {
		err = atm_.err;
		atm_.close();
		return false;
	}

	return true;
}

bool GConvertEthAutoMac::doClose() {
	bool res = GPcapDeviceWrite::doClose();
	atm_.close();
	return res;
}

GMac GConvertEthAutoMac::resolveMacByDip(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) {
		SET_ERR(GErr::VALUE_IS_NULL, "ip header is null");
		return GMac::nullMac();
	}
	GIp dip = ipHdr->dip();
	if (dip == myIp_) return myMac_;
	GIp adjIp = intf_->getAdjIp(dip);
	GAtmMap::iterator it = atm_.find(adjIp);
	if (it == atm_.end()) {
		atm_.insert(adjIp, GMac::nullMac());
		bool res = atm_.wait();
		if (!res) {
			qCritical() << QString("can not resolve mac for ip %1").arg(QString(adjIp));
			return GMac::nullMac();
		}
		if (!active()) {
			QString msg = QString("not opened state %1").arg(int(state_));
			SET_ERR(GErr::NOT_OPENED_STATE, msg); // gilgil temp 2019.06.02
			return GMac::nullMac();
		}
	}
	return it.value();
}

void GConvertEthAutoMac::convert(GPacket* packet) {
	GPacket::Dlt dlt = packet->dlt();
	switch (dlt) {
		case GPacket::Eth: {
			emit converted(packet);
			break;
		}
		case GPacket::Ip: {
			GEthHdr* ethHdr = PEthHdr(convertedEthBuf_);
			ethHdr->smac_ = myMac_;
			ethHdr->dmac_ = resolveMacByDip(packet);
			ethHdr->type_ = htons(type_);

			size_t copyLen = packet->buf_.size_;
			memcpy(convertedEthBuf_ + sizeof(GEthHdr), packet->buf_.data_, copyLen);
			convertedEthPacket_.copyFrom(packet, GBuf(convertedEthBuf_, sizeof(GEthHdr) + copyLen));
			emit converted(&convertedEthPacket_);
			break;
		}
		case GPacket::Dot11:
		case GPacket::Null: {
			QString msg = QString("not supported data link type(%1)").arg(GPacket::dltToString(dlt));
			SET_ERR(GErr::NOT_SUPPORTED, msg);
			return;
		}
	}
}
