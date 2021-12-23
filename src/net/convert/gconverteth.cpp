#include "gconverteth.h"

// ----------------------------------------------------------------------------
// GConvertEth
// ----------------------------------------------------------------------------
bool GConvertEth::doOpen() {
	Q_ASSERT(convertedEthBuf_ == nullptr);
	convertedEthBuf_ = new gbyte[bufSize_];
	return true;
}

bool GConvertEth::doClose() {
	if (convertedEthBuf_ != nullptr) {
		delete[] convertedEthBuf_;
		convertedEthBuf_ = nullptr;
	}
	return true;
}

void GConvertEth::convert(GPacket* packet) {
	GPacket::Dlt dlt = packet->dlt();
	switch (dlt) {
		case GPacket::Eth: {
			emit converted(packet);
			break;
		}
		case GPacket::Ip: {
			GEthHdr* ethHdr = PEthHdr(convertedEthBuf_);
			ethHdr->smac_ = smac_;
			ethHdr->dmac_ = dmac_;
			ethHdr->type_ = htons(type_);

			size_t copyLen = packet->buf_.size_;
			if (int(copyLen) > bufSize_) {
				qWarning() << QString("copyLen(%1) > bufSize_(%2)").arg(copyLen).arg(bufSize_);
				return;
			}
			memcpy(convertedEthBuf_ + sizeof(GEthHdr), packet->buf_.data_, copyLen);
			convertedEthPacket_.copyFrom(packet, GBuf(convertedEthBuf_, sizeof(GEthHdr) + copyLen));
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
