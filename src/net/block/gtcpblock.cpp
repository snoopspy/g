#include "gtcpblock.h"
#include "net/write/gpcapdevicewrite.h"
#include <cstring> // for memcpy

// ----------------------------------------------------------------------------
// GTcpBlock
// ----------------------------------------------------------------------------
bool GTcpBlock::doOpen() {
	if (writer_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "writer must be specified");
		return false;
	}

	forwardFinMsgStr_= forwardFinMsg_.join("\r\n");
	backwardFinMsgStr_ = backwardFinMsg_.join("\r\n");

	Q_ASSERT(blockBuf_ == nullptr);
	blockBuf_ = new gbyte[bufSize_];

	return true;
}

bool GTcpBlock::doClose() {
	if (blockBuf_ != nullptr) {
		delete[] blockBuf_;
		blockBuf_ = nullptr;
	}

	return true;
}

void GTcpBlock::sendBlockPacket(GPacket* packet, GTcpBlock::Direction direction, GTcpBlock::BlockType blockType, uint32_t seq, uint32_t ack, QString msg) {
	GPacket* blockPacket = nullptr;
	size_t copyLen;

	GPacket::Dlt dlt = packet->dlt();
	switch (dlt) {
		case GPacket::Eth: blockPacket = &blockEthPacket_; copyLen = sizeof(GEthHdr) + sizeof(GIpHdr) + sizeof(GTcpHdr); break;
		case GPacket::Ip: blockPacket = &blockIpPacket_; copyLen = sizeof(GIpHdr) + sizeof(GTcpHdr); break;
		case GPacket::Dot11: blockPacket = nullptr; break;
		case GPacket::Null: blockPacket = nullptr; break;
	}
	if (blockPacket == nullptr) {
		SET_ERR(GErr::NotSupported, QString("Not supported dlt(%d)").arg(GPacket::dltToInt(dlt)));
		return;
	}

	if (int(copyLen) > bufSize_) {
		qWarning() << QString("copyLen(%1) > bufSize_(%2)").arg(copyLen).arg(bufSize_);
		return;
	}
	memcpy(blockBuf_, packet->buf_.data_, copyLen);
	blockPacket->copyFrom(packet, GBuf(blockBuf_, copyLen));

	GTcpHdr* tcpHdr = blockPacket->tcpHdr_;
	Q_ASSERT(tcpHdr != nullptr);

	//
	// Data
	//
	if (blockType == Fin) {
		blockPacket->tcpData_.data_ = pbyte(tcpHdr) + sizeof(GTcpHdr);
		memcpy(blockPacket->tcpData_.data_, qPrintable(msg), msg.size());
		blockPacket->tcpData_.size_ = msg.size();
	}

	//
	// TCP
	//
	if (direction == Backward)
		std::swap(tcpHdr->sport_, tcpHdr->dport_);
	tcpHdr->seq_ = htonl(seq);
	tcpHdr->ack_ = htonl(ack);
	if (blockType == Rst) {
		tcpHdr->flags_ = GTcpHdr::Rst | GTcpHdr::Ack;
		tcpHdr->win_ = 0;
	} else {
		tcpHdr->flags_ = GTcpHdr::Fin | GTcpHdr::Ack | GTcpHdr::Psh;
	}
	tcpHdr->off_rsvd_ = (sizeof(GTcpHdr) / 4) << 4;
	tcpHdr->flags_ &= ~GTcpHdr::Syn;

	//
	// IP
	//
#ifndef Q_OS_ANDROID
	GIpHdr* ipHdr = blockPacket->ipHdr_;
#else
	GVolatileIpHdr* ipHdr = PVolatileIpHdr(blockPacket->ipHdr_);
#endif
	Q_ASSERT(ipHdr != nullptr);
	if (blockType == Rst)
		ipHdr->len_ = htons(sizeof(GIpHdr) + sizeof(GTcpHdr));
	else
		ipHdr->len_ = htons(sizeof(GIpHdr) + sizeof(GTcpHdr) + msg.size());
	ipHdr->tos_ = 0x44;
	if (direction == Backward) {
		ipHdr->ttl_ = 0x80;
		std::swap(ipHdr->sip_, ipHdr->dip_);
	}

	//
	// checksum
	//
	tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(PIpHdr(ipHdr), tcpHdr));
	ipHdr->sum_ = htons(GIpHdr::calcChecksum(PIpHdr(ipHdr)));

	//
	// Ethernet
	//
	GPcapDeviceWrite* pcapDeviceWrite = dynamic_cast<GPcapDeviceWrite*>(writer_);
	if (pcapDeviceWrite != nullptr && packet->ethHdr_ != nullptr) {
		GEthHdr* ethHdr = blockPacket->ethHdr_;
		Q_ASSERT(pcapDeviceWrite->intf() != nullptr);
		GMac myMac = pcapDeviceWrite->intf()->mac();
		if (direction == Backward) {
			ethHdr->dmac_ = ethHdr->smac();
			ethHdr->smac_ = myMac;
		} else {
			//ethHdr->dmac_ = ethHdr->dmac();
			ethHdr->smac_ = myMac;
		}
	}

	// buf size
	size_t bufSize = 0;
	if (dlt == GPacket::Eth) bufSize += sizeof(GEthHdr);
	bufSize += sizeof(GIpHdr) + sizeof(GTcpHdr);
	if (blockType == Fin) bufSize += msg.size();
	blockPacket->buf_.size_ = bufSize;

	// write
	writer_->write(blockPacket);
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
