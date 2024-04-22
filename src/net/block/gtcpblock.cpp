#include "gtcpblock.h"

// ----------------------------------------------------------------------------
// GTcpBlock
// ----------------------------------------------------------------------------
bool GTcpBlock::doOpen() {
	if (!enabled_) return true;

	if (objectName() != "") writer_.setObjectName(objectName() + ".writer_");
	if (!writer_.open()) {
		err = writer_.err;
		return false;
	}

	forwardFinMsgStr_= forwardFinMsg_.join("\r\n");
	backwardFinMsgStr_ = backwardFinMsg_.join("\r\n");

	return true;
}

bool GTcpBlock::doClose() {
	if (!enabled_) return true;

	return writer_.close();
}

void GTcpBlock::sendBlockPacket(GPacket* packet, GTcpBlock::Direction direction, GTcpBlock::BlockType blockType, uint32_t seq, uint32_t ack, QString msg) {
	size_t blockLen = sizeof(GIpHdr) + sizeof(GTcpHdr) + msg.size();
	blockByteArray_.resize(blockLen);

	//
	// blockIpPacket_
	//
	gbyte* data = pbyte(blockByteArray_.data());
	blockIpPacket_.clear();
	blockIpPacket_.buf_.data_ = data;
	blockIpPacket_.buf_.size_ = blockLen;
	blockIpPacket_.ipHdr_ = PIpHdr(data);
	blockIpPacket_.tcpHdr_ = PTcpHdr(data + sizeof(GIpHdr));
	if (blockType == Fin) {
		blockIpPacket_.tcpData_.data_ = data + sizeof(GIpHdr) + sizeof(GTcpHdr);
		blockIpPacket_.tcpData_.size_ = msg.size();
	} else {
		blockIpPacket_.tcpData_.clear();
	}

	//
	// IP
	//
	GIpHdr* blockIpHdr = blockIpPacket_.ipHdr_;
	GIpHdr* orgIpHdr = packet->ipHdr_; // Should disable compile optimization for GIPHdr(sip_ and dip_)
	Q_ASSERT(orgIpHdr != nullptr);
	memcpy(pvoid(blockIpHdr), pvoid(orgIpHdr), sizeof(GIpHdr));
	blockIpHdr->v_hlen_ = 0x45;
	blockIpHdr->tos_ = 0x44;
	if (blockType == Rst)
		blockIpHdr->tlen_ = htons(sizeof(GIpHdr) + sizeof(GTcpHdr));
	else
		blockIpHdr->tlen_ = htons(sizeof(GIpHdr) + sizeof(GTcpHdr) + msg.size());
	blockIpHdr->id_ = 0;
	if (direction == Backward) {
		blockIpHdr->ttl_ = 0x80;
		std::swap(blockIpHdr->sip_, blockIpHdr->dip_);
	}

	//
	// TCP
	//
	GTcpHdr* blockTcpHdr = blockIpPacket_.tcpHdr_;
	GTcpHdr* orgTcpHdr = packet->tcpHdr_;
	Q_ASSERT(orgTcpHdr != nullptr);
	memcpy(blockTcpHdr, orgTcpHdr, sizeof(GTcpHdr));
	if (direction == Backward)
		std::swap(blockTcpHdr->sport_, blockTcpHdr->dport_);
	blockTcpHdr->seq_ = htonl(seq);
	blockTcpHdr->ack_ = htonl(ack);
	if (blockType == Rst) {
		blockTcpHdr->flags_ = GTcpHdr::Rst | GTcpHdr::Ack;
		blockTcpHdr->win_ = 0;
	} else {
		blockTcpHdr->flags_ = GTcpHdr::Fin | GTcpHdr::Ack | GTcpHdr::Psh;
	}
	blockTcpHdr->off_rsvd_ = (sizeof(GTcpHdr) / 4) << 4;
	blockTcpHdr->flags_ &= ~GTcpHdr::Syn;

	//
	// Data
	//
	if (blockType == Fin) {
		memcpy(blockIpPacket_.tcpData_.data_, qPrintable(msg), msg.size());
	}

	//
	// checksum
	//
	blockIpHdr->sum_ = htons(GIpHdr::calcChecksum(blockIpHdr));
	blockTcpHdr->sum_ = htons(GTcpHdr::calcChecksum(blockIpHdr, blockTcpHdr));

	// write
	writer_.write(&blockIpPacket_);
}

void GTcpBlock::block(GPacket* packet) {
	if (!enabled_) return;

	if (packet->tcpHdr_ == nullptr) return;

	GPacket::Dlt dlt = packet->dlt();
	if (dlt != GPacket::Eth && dlt != GPacket::Ip) {
		qWarning() << QString("not supported dlt(%1)").arg(GPacket::dltToString(dlt));
		return;
	}

	bool synExist = (packet->tcpHdr_->flags() & GTcpHdr::Syn) != 0;
	bool rstExist = (packet->tcpHdr_->flags() & GTcpHdr::Rst) != 0;
	bool finExist = (packet->tcpHdr_->flags() & GTcpHdr::Fin) != 0;
	bool ackExist = (packet->tcpHdr_->flags() & GTcpHdr::Ack) != 0;
	if (rstExist || finExist) return;

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	Q_ASSERT(tcpHdr != nullptr);

	size_t tcpDataSize = packet->tcpData_.size_;
	uint32_t seq = tcpHdr->seq();
	uint32_t nextSeq = seq + tcpDataSize + (synExist || finExist ? 1 : 0);
	uint32_t ack = tcpHdr->ack();

	bool _blocked = false;
	if (forwardBlockType_ == Rst) {
		if (synExist && !ackExist)
			sendBlockPacket(packet, Forward, Rst, seq, 0); // useless
		else
			sendBlockPacket(packet, Forward, Rst, nextSeq, ack);
		_blocked = true;
	}
	if (forwardBlockType_ == Fin) {
		if (!synExist) {
			sendBlockPacket(packet, Forward, Fin, nextSeq, ack, forwardFinMsgStr_);
			_blocked = true;
		}
	}
	if (backwardBlockType_ == Rst) {
		if (synExist && !ackExist)
			sendBlockPacket(packet, Backward, Rst, 0, nextSeq);
		else
			sendBlockPacket(packet, Backward, Rst, ack, nextSeq);
		_blocked = true;
	}
	if (backwardBlockType_ == Fin) {
		if (!synExist) {
			sendBlockPacket(packet, Backward, Fin, ack, nextSeq, backwardFinMsgStr_);
			_blocked = true;
		}
	}

	if (_blocked)
		emit blocked(packet);

	if (forwardBlockType_ != None)
		packet->ctrl_.block_ = true;
}
