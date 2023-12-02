#include "gconverteth.h"

// ----------------------------------------------------------------------------
// GConvertEth
// ----------------------------------------------------------------------------
void GConvertEth::convert(GPacket* packet) {
	GPacket::Dlt dlt = packet->dlt();
	switch (dlt) {
		case GPacket::Eth: {
			emit converted(packet);
			break;
		}
		case GPacket::Ip: {
			size_t copyLen = packet->buf_.size_;
			convertedByteArray_.resize(sizeof(GEthHdr) + copyLen);

			GEthHdr* ethHdr = PEthHdr(convertedByteArray_.data());
			ethHdr->smac_ = smac_;
			ethHdr->dmac_ = dmac_;
			ethHdr->type_ = htons(type_);

			memcpy(convertedByteArray_.data() + sizeof(GEthHdr), packet->buf_.data_, copyLen);
			GBuf buf(pbyte(convertedByteArray_.data()), sizeof(GEthHdr) + copyLen);
			convertedEthPacket_.copyFrom(packet, buf);
			emit converted(&convertedEthPacket_);
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
