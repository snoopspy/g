#include "gconvertip.h"

// ----------------------------------------------------------------------------
// GConvertIp
// ----------------------------------------------------------------------------
void GConvertIp::convert(GPacket* packet) {
	GPacket::Dlt dlt = packet->dlt();
	switch (dlt) {
		case GPacket::Eth: {
			size_t copyLen = packet->buf_.size_ - sizeof(GEthHdr);
			convertedByteArray_.resize(copyLen);
			memcpy(convertedByteArray_.data(), packet->buf_.data_ + sizeof(GEthHdr), copyLen);
			GBuf buf(pbyte(convertedByteArray_.data()), copyLen);
			convertedIpPacket_.copyFrom(packet, buf);
			emit converted(&convertedIpPacket_);
			break;
		}
		case GPacket::Ip: {
			emit converted(packet);
			break;
		}
		case GPacket::Dot11:
		case GPacket::Null: {
			QString msg = QString("not supported data link type(%1)").arg(GPacket::dltToString(dlt));
			SET_ERR(GErr::NotSupported, msg);
			return;
		}
	}
}
