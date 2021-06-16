#include "gconvertip.h"

// ----------------------------------------------------------------------------
// GConvertIp
// ----------------------------------------------------------------------------
bool GConvertIp::doOpen() {
	Q_ASSERT(convertedIpBuf_ == nullptr);
	convertedIpBuf_ = new gbyte[bufSize_];
	return true;
}

bool GConvertIp::doClose() {
	if (convertedIpBuf_ != nullptr) {
		delete[] convertedIpBuf_;
		convertedIpBuf_ = nullptr;
	}
	return true;
}

void GConvertIp::convert(GPacket* packet) {
	GPacket::Dlt dlt = packet->dlt();
	switch (dlt) {
		case GPacket::Eth: {
			size_t copyLen = packet->buf_.size_ - sizeof(GEthHdr);
			if ((int)copyLen > bufSize_) {
				qWarning() << QString("copyLen(%1) > bufSize_(%2)").arg(copyLen).arg(bufSize_);
				return;
			}
			memcpy(convertedIpBuf_, packet->buf_.data_ + sizeof(GEthHdr), copyLen);
			convertedIpPacket_.copyFrom(packet, GBuf(convertedIpBuf_, copyLen));
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
			SET_ERR(GErr::NOT_SUPPORTED, msg);
			return;
		}
	}
}
