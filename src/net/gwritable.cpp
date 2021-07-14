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

	gbyte* tcpDataData = tcpData.data_;
	//gbyte* tcpDataData = pbyte(tcpHdr) + 20;
	//tcpHdr->off_rsvd_ = 0x50; // Set tcp header size 20 bytes

	size_t ipTcpHdrSize = (ipHdr->hl() + tcpHdr->off()) * 4;
	size_t totalTcpDataSize = packet->buf_.size_ - (sizeof(GEthHdr) + ipTcpHdrSize);
	while (true) {
		if (ipTcpHdrSize + totalTcpDataSize <= GPacket::MtuSize) break;

		packet->buf_.size_ = sizeof(GEthHdr) + GPacket::MtuSize;
		size_t onceTcpDataSize = GPacket::MtuSize - ipTcpHdrSize;
		qDebug() << "onceTcpDataSize =" << onceTcpDataSize; // gilgil temp 2021.07.10
		tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		write(packet->buf_);
		QThread::msleep(10); // gilgil temp 2021.07.10

		tcpHdr->seq_ = htonl(tcpHdr->seq() + onceTcpDataSize); // next seq
		memcpy(tcpDataData, tcpDataData + onceTcpDataSize, sizeof(GEthHdr) + GPacket::MtuSize); // next data
		totalTcpDataSize -= onceTcpDataSize;
		// QThread::msleep(1); // gilgil temp 2021.07.12
	}
	qDebug() << "lastTcpDataSize =" << totalTcpDataSize; // gilgil temp 2021.07.10
	ipHdr->len_ = ntohs(ipTcpHdrSize + totalTcpDataSize);
	ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));
	tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
	packet->buf_.size_ = sizeof(GEthHdr) + ipTcpHdrSize + totalTcpDataSize;
	GPacket::Result res = write(packet->buf_); // gilgil temp 2021.07.10

	packet->buf_ = backupBuf;
	memcpy(packet->buf_.data_, backupData, packet->buf_.size_);
	delete[] backupData;
	return res;
}

// ----- gilgi temp 2021.07.12 -----
/*
#include "net/write/grawipsocketwrite.h"
GPacket::Result GWritable::writeMtuSplit(GPacket* packet) {
	GRawIpSocketWrite write;
	Q_ASSERT(write.open());
	GBuf backupBuf = packet->buf_;
	packet->buf_.data_ += sizeof(GEthHdr);
	packet->buf_.size_ -= sizeof(GEthHdr);
	GPacket::Result res = write.write(packet);
	packet->buf_ = backupBuf;

	write.close();
	return res;
}
*/
// ---------------------------------

