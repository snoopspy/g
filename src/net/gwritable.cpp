#include "gwritable.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GWritable
// ----------------------------------------------------------------------------
GPacket::Result GWritable::writeMtuSplit(GPacket* packet) {
	qDebug() << ""; // gilgil temp 2021.07.10

	GEthHdr* ethHdr = packet->ethHdr_;
	if (ethHdr == nullptr) {
		qWarning() << "ethHdr is null";
		return GPacket::Fail;
	}

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) {
		qWarning() << "ipHdr is null";
		return GPacket::Fail;
	}

	GTcpHdr* tcpHdr = packet->tcpHdr_;
	if (tcpHdr == nullptr) {
		qWarning() << "tcpHdr is null";
		return GPacket::Fail;
	}

	GBuf tcpData = packet->tcpData_;
	if (tcpData.data_ == nullptr) {
		qWarning() << "tcpData is null";
		return GPacket::Fail;
	}

	GBuf backupBuf = packet->buf_;
	gbyte* backupData = new gbyte[packet->buf_.size_];
	memcpy(backupData, packet->buf_.data_, packet->buf_.size_);

	size_t ipTcpHdrSize = (ipHdr->hl() + tcpHdr->off()) * 4;
	while (true) {
		size_t size = packet->buf_.size_;
		if (size <= sizeof(GEthHdr) + GPacket::MtuSize) break;
		packet->buf_.size_ = sizeof(GEthHdr) + GPacket::MtuSize;
		size_t tcpDataSize = GPacket::MtuSize - ipTcpHdrSize;
		ipHdr->len_ = ntohs(ipTcpHdrSize + tcpDataSize);
		tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
		GPacket::Result res = write(packet->buf_);
		if (res != GPacket::Ok) {
			delete[] backupData;
			return res;
		}
		tcpHdr->seq_ = htonl(tcpHdr->seq() + tcpDataSize);
		memcpy(tcpData.data_, tcpData.data_ + tcpDataSize, sizeof(GEthHdr) + GPacket::MtuSize);
		packet->buf_.size_ = size - (sizeof(GEthHdr) + GPacket::MtuSize);
	}
	GPacket::Result res = write(packet->buf_);
	packet->buf_ = backupBuf;
	memcpy(packet->buf_.data_, backupData, packet->buf_.size_);
	delete[] backupData;
	return res;
}
