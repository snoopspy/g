#include "gclienthellosplit.h"

// ----------------------------------------------------------------------------
// GClientHelloSplit
// ----------------------------------------------------------------------------
bool GClientHelloSplit::doOpen() {
	if (tcpFlowMgr_ == nullptr) {
		SET_ERR(GErr::OBJECT_IS_NULL, "tcpFlowMgr must be specified");
		return false;
	}
	tcpFlowOffset_ = tcpFlowMgr_->requestItems_.request("GFlowMgrTest_tcp", sizeof(FlowItem));
	tcpFlowMgr_->managables_.insert(this);

	return true;
}

bool GClientHelloSplit::doClose() {
	return true;
}

void GClientHelloSplit::tcpFlowCreated(GFlow::TcpFlowKey* key, GFlow::Value* value) {
	// qDebug() << QString("_tcpFlowCreated %1:%2>%3:%4").arg(QString(key->sip_), QString::number(key->sport_), QString(key->dip_), QString::number(key->dport_)); // gilgil temp 2021.04.07
	(void)key;
	FlowItem* flowItem = PFlowItem(value->mem(tcpFlowOffset_));
	new (flowItem) FlowItem;
}

void GClientHelloSplit::tcpFlowDeleted(GFlow::TcpFlowKey* key, GFlow::Value* value) {
	(void)key;
	(void)value;
	// qDebug() << QString("_tcpFlowDeleted %1:%2>%3:%4").arg(QString(key->sip_), QString::number(key->sport_), QString(key->dip_), QString::number(key->dport_)); // gilgil temp 2021.04.07
}

void GClientHelloSplit::split(GPacket* packet) {
	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) return;
	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) return;

	Q_ASSERT(tcpFlowMgr_->val_ != nullptr);
	FlowItem* flowItem = PFlowItem(tcpFlowMgr_->val_->mem(tcpFlowOffset_));
	if (flowItem->processed_) return;

	GBuf tcpData = packet->tcpData_;
	if (!tcpData.valid()) return; // no tcp data
	flowItem->processed_ = true;

	if (tcpData.size_ <= 16) return; // too small data

	if (tcpData.data_[0] != 0x16 || tcpData.data_[1] != 0x03) return;

	GFlow::TcpFlowKey* key = &tcpFlowMgr_->key_;
	qDebug() << QString("split!!! tcp size=%1 %2:%3>%4:%5")
		.arg(packet->buf_.size_)
		.arg(QString(key->sip_)).arg(key->sport_).arg(QString(key->dip_)).arg(key->dport_); // gilgil temp 2016.10.10

	size_t orgDataSize = tcpData.size_;
	size_t firstDataSize = 16;
	size_t secondDataSize = orgDataSize - firstDataSize;

	Q_ASSERT(tcpData.size_ < GPacket::MaxBufSize);
	memcpy(splittedTcpData_, tcpData.data_, tcpData.size_);

	{
		//
		// first 16 bytes
		//
		GBuf backup = packet->buf_;
		uint16_t oldIpHdrLen = ipHdr->len();
		uint16_t newIpHdrLen = ipHdr->hl() * 4 + tcpHdr->off() * 4 + uint16_t(firstDataSize);
		ssize_t ipHdrLenDiff = newIpHdrLen - oldIpHdrLen;
		ipHdr->len_ = htons(newIpHdrLen);
		tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
		packet->buf_.size_ += ipHdrLenDiff;
		emit splitted(packet);
		packet->buf_ = backup;
	}

	{
		//
		// second extra bytes
		//
		GBuf backup = packet->buf_;
		uint16_t oldIpHdrLen = ipHdr->len();
		uint16_t newIpHdrLen = ipHdr->hl() * 4 + tcpHdr->off() * 4 + uint16_t(secondDataSize);
		ssize_t ipHdrLenDiff = newIpHdrLen - oldIpHdrLen;
		ipHdr->len_ = htons(newIpHdrLen);
		memcpy(tcpData.data_, splittedTcpData_ + firstDataSize, secondDataSize);
		tcpHdr->seq_ = htonl(tcpHdr->seq() + 16);
		tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
		packet->buf_.size_ += ipHdrLenDiff;
		emit splitted(packet);
		packet->buf_ = backup;
	}

	packet->ctrl.block_ = true;
}
