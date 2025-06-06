#include "gudpblock.h"

// ----------------------------------------------------------------------------
// GUdpBlock
// ----------------------------------------------------------------------------
bool GUdpBlock::doOpen() {
	if (!enabled_) return true;

	if (objectName() != "") writer_.setObjectName(objectName() + ".writer_");
	if (!writer_.open()) {
		err = writer_.err;
		return false;
	}

	return true;
}

bool GUdpBlock::doClose() {
	if (!enabled_) return true;

	return writer_.close();
}

void GUdpBlock::block(GPacket* packet) {
	if (!enabled_) return;

	if (packet->udpHdr_ == nullptr) return;

	GPacket::Dlt dlt = packet->dlt();
	if (dlt != GPacket::Eth && dlt != GPacket::Ip) {
		qWarning() << QString("not supported dlt(%1)").arg(GPacket::dltToString(dlt));
		return;
	}

	GIpHdr* orgIpHdr = packet->ipHdr_; // Should disable compile optimization for GIPHdr(sip_ and dip_)
	Q_ASSERT(orgIpHdr != nullptr);

	if (orgIpHdr->dip().isBroadcast() || orgIpHdr->dip().isMulticast()) return;

	int orgIpHdrLen = packet->ipHdr_->hlen() * 4;
	int orgUdpHdrLen = sizeof(GUdpHdr);
	int orgPayloadLen = packet->ipHdr_->tlen() - orgIpHdrLen - orgUdpHdrLen;

	int blockLen = sizeof(GIpHdr) + sizeof(GIcmpIpHdr) + orgIpHdrLen;
	switch (blockType_) {
		case L2: break;
		case L3: blockLen += orgUdpHdrLen; break;
		case Payload: blockLen += orgUdpHdrLen + orgPayloadLen; break;
	}
	if (blockLen > maxBlockSize_)
		blockLen = maxBlockSize_;

	blockByteArray_.resize(blockLen);

	//
	// blockIpPacket_
	//
	gbyte* data = pbyte(blockByteArray_.data());
	blockIpPacket_.clear();
	blockIpPacket_.buf_.data_ = data;
	blockIpPacket_.buf_.size_ = blockLen;
	blockIpPacket_.ipHdr_ = PIpHdr(data);
	blockIpPacket_.icmpHdr_ = PIcmpHdr(data + sizeof(GIpHdr));

	//
	// IP
	//
	GIpHdr* blockIpHdr = blockIpPacket_.ipHdr_;
	memcpy(pvoid(blockIpHdr), pvoid(orgIpHdr), sizeof(GIpHdr));
	blockIpHdr->v_hlen_ = 0x45;
	blockIpHdr->tos_ = 0x44;
	blockIpHdr->tlen_ = htons(blockLen);
	blockIpHdr->id_ = 0;
	blockIpHdr->p_ = GIpHdr::Icmp;
	blockIpHdr->sum_ = 0;

	std::swap(blockIpHdr->sip_, blockIpHdr->dip_);

	//
	// ICMP
	//
	GIcmpIpHdr* blockIcmpIpHdr = static_cast<GIcmpIpHdr*>(blockIpPacket_.icmpHdr_);
	blockIcmpIpHdr->type_ = 3; // Destination unreachable
	blockIcmpIpHdr->code_ = 3; // Port unreachable
	blockIcmpIpHdr->sum_ = htons(0); // gilgil temp 2024.04.12
	blockIcmpIpHdr->unused_ = 0;

	//
	// OrgIpHdr
	//
	memcpy(data + sizeof(GIpHdr) + sizeof(GIcmpIpHdr), orgIpHdr, blockLen - sizeof(GIpHdr) - sizeof(GIcmpIpHdr));

	//
	// checksum
	//
	blockIpHdr->sum_ = htons(GIpHdr::calcSum(blockIpHdr));
	blockIcmpIpHdr->sum_ = htons(GIcmpHdr::calcSum(blockIpHdr, blockIcmpIpHdr));

	writer_.write(&blockIpPacket_);
	emit blocked(packet);
	packet->ctrl_.block_ = true;
}
