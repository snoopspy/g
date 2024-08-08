#include "gclienthellosplit.h"

// ----------------------------------------------------------------------------
// GClientHelloSplit
// ----------------------------------------------------------------------------
bool GClientHelloSplit::doOpen() {
	if (tcpFlowMgr_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "tcpFlowMgr is null");
		return false;
	}
	tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request(this, sizeof(Item));
	tcpFlowMgr_->managables_.insert(this);

	if (!writer_.open()) {
		err = writer_.err;
		return false;
	}

	Q_ASSERT(splittedTcpDataBuf_ == nullptr);
	splittedTcpDataBuf_ = new gbyte[bufSize_];

	return true;
}

bool GClientHelloSplit::doClose() {
	if (splittedTcpDataBuf_ != nullptr) {
		delete[] splittedTcpDataBuf_;
		splittedTcpDataBuf_ = nullptr;
	}

	writer_.close();

	return true;
}

void GClientHelloSplit::tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	// qDebug() << QString("_tcpFlowDetected %1").arg(QString(tcpFlowKey)); // gilgil temp 2021.04.07
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	new (item) Item;
}

void GClientHelloSplit::tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) {
	(void)tcpFlowKey;
	Item* item = getItem(tcpFlowValue);
	item->~Item();
	// qDebug() << QString("_tcpFlowDeleted %1").arg(QString(tcpFlowKey)); // gilgil temp 2021.04.07
}

void GClientHelloSplit::split(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) return;

	Q_ASSERT(tcpFlowMgr_->currentTcpFlowVal_ != nullptr);
	Item* item = getItem(tcpFlowMgr_->currentTcpFlowVal_);
	if (item->processed_) return;

	if (tcpHdr->flags() & GTcpHdr::Syn && tcpHdr->flags() & GTcpHdr::Ack) {
		item->processed_ = true;
		return;
	}

	GBuf tcpData = packet->tcpData_;
	if (!tcpData.valid()) return; // no tcp data
	item->processed_ = true;

	if (tcpData.size_ <= 16) return; // too small data

	if (tcpData.data_[0] != 0x16 || tcpData.data_[1] != 0x03) return;

	GFlow::TcpFlowKey* tcpFlowKey = &tcpFlowMgr_->currentTcpFlowKey_;
	qDebug() << QString("tcp size=%1 %2").arg(packet->buf_.size_).arg(QString(*tcpFlowKey)); // gilgil temp 2016.10.10

	size_t orgDataSize = tcpData.size_;
	size_t firstDataSize = 16;
	size_t secondDataSize = orgDataSize - firstDataSize;

	if (int(tcpData.size_) > bufSize_) {
		qWarning() << QString("tcpData.size_(%1) > bufSize_(%2").arg(tcpData.size_).arg(bufSize_);
		return;
	}
	memcpy(splittedTcpDataBuf_, tcpData.data_, tcpData.size_);

	{
		//
		// first 16 bytes
		//
		GBuf backup = packet->buf_;
		uint16_t oldIpHdrLen = ipHdr->tlen();
		uint16_t newIpHdrLen = ipHdr->hlen() * 4 + tcpHdr->off() * 4 + uint16_t(firstDataSize);
		ssize_t ipHdrLenDiff = newIpHdrLen - oldIpHdrLen;
		ipHdr->tlen_ = htons(newIpHdrLen);
		tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
		packet->buf_.size_ += ipHdrLenDiff;
		GPacket::Result res = writer_.write(packet);
		if (res != GPacket::Ok) {
			qWarning() << "writer_.write return" << int(res);
		}
		packet->buf_ = backup;
	}

	{
		//
		// second extra bytes
		//
		GBuf backup = packet->buf_;
		uint16_t oldIpHdrLen = ipHdr->tlen();
		uint16_t newIpHdrLen = ipHdr->hlen() * 4 + tcpHdr->off() * 4 + uint16_t(secondDataSize);
		ssize_t ipHdrLenDiff = newIpHdrLen - oldIpHdrLen;
		ipHdr->tlen_ = htons(newIpHdrLen);
		memcpy(tcpData.data_, splittedTcpDataBuf_ + firstDataSize, secondDataSize);
		tcpHdr->seq_ = htonl(tcpHdr->seq() + 16);
		tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
		packet->buf_.size_ += ipHdrLenDiff;
		GPacket::Result res = writer_.write(packet);
		if (res != GPacket::Ok) {
			qWarning() << "writer_.write return" << int(res);
		}
		packet->buf_ = backup;
	}

	packet->ctrl_.block_ = true;
}
