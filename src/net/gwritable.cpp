#include "gwritable.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GWritable
// ----------------------------------------------------------------------------
GPacket::Result GWritable::writeMtuSplit(GPacket* packet) {
	qDebug() << packet->buf_.size_; // gilgil temp 2021.07.10

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

	ipHdr->len_ = ntohs(GPacket::MtuSize);
	ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));

	size_t ipTcpHdrSize = (ipHdr->hl() + tcpHdr->off()) * 4;
	size_t totalTcpDataSize = packet->buf_.size_ - (sizeof(GEthHdr) + ipTcpHdrSize);
	while (true) {
		if (ipTcpHdrSize + totalTcpDataSize <= GPacket::MtuSize) break;

		packet->buf_.size_ = sizeof(GEthHdr) + GPacket::MtuSize;
		size_t onceTcpDataSize = GPacket::MtuSize - ipTcpHdrSize;
		qDebug() << "onceTcpDataSize =" << onceTcpDataSize; // gilgil temp 2021.07.10
		tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		write(packet->buf_);
		//QThread::msleep(10); // gilgil temp 2021.07.10

		tcpHdr->seq_ = htonl(tcpHdr->seq() + onceTcpDataSize); // next seq
		memcpy(tcpData.data_, tcpData.data_ + onceTcpDataSize, sizeof(GEthHdr) + GPacket::MtuSize);
		totalTcpDataSize -= onceTcpDataSize;
	}
	qDebug() << "lastTcpDataSize =" << totalTcpDataSize; // gilgil temp 2021.07.10
	ipHdr->len_ = ntohs(ipTcpHdrSize + totalTcpDataSize);
	ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
	packet->buf_.size_ = sizeof(GEthHdr) + ipTcpHdrSize + totalTcpDataSize;
	GPacket::Result res = write(packet->buf_);

	packet->buf_ = backupBuf;
	memcpy(packet->buf_.data_, backupData, packet->buf_.size_);
	delete[] backupData;
	return res;
}
