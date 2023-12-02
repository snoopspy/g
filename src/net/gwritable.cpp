#include "gwritable.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GWritable
// ----------------------------------------------------------------------------
GPacket::Result GWritable::writeMtuSplit(GPacket* packet, size_t mtu, GPacket::Dlt dlt, GDuration usleepTime) {
	size_t ethernetSize = 0; // for remove warning
	switch (dlt) {
		case GPacket::Eth: ethernetSize = sizeof(GEthHdr); break;
		case GPacket::Ip: ethernetSize = 0; break;
		case GPacket::Dot11:
		case GPacket::Null:
			qCritical() << QString("invalid datalinktype %1").arg(int(dlt));
			return GPacket::Fail;
	}
	GPacket* sendPacket = anyPacket_.get(dlt);
	Q_ASSERT(sendPacket != nullptr);

	sendByteArray_.resize(packet->buf_.size_);
	memcpy(sendByteArray_.data(), packet->buf_.data_, packet->buf_.size_);
	GBuf buf(pbyte(sendByteArray_.data()), packet->buf_.size_);
	sendPacket->copyFrom(packet, buf);

	GIpHdr* ipHdr = sendPacket->ipHdr_;
	if (ipHdr == nullptr) {
		qWarning() << "ipHdr is null";
		return GPacket::Fail;
	}

	GTcpHdr* tcpHdr = sendPacket->tcpHdr_;
	if (tcpHdr == nullptr) {
		qWarning() << "tcpHdr is null";
		return GPacket::Fail;
	}

	GBuf tcpData = sendPacket->tcpData_;
	if (tcpData.data_ == nullptr) {
		qWarning() << "tcpData is null";
		return GPacket::Fail;
	}

	size_t ipTcpHdrSize = (ipHdr->hlen() + tcpHdr->off()) * 4;
	size_t tcpDataSize = ipHdr->tlen() - ipTcpHdrSize;

	gbyte* tcpDataData = tcpData.data_;
	GPacket::Result result = GPacket::Ok;

	while (tcpDataSize > 0) {
		size_t onceTcpDataSize = tcpDataSize;
		if (onceTcpDataSize > mtu - ipTcpHdrSize)
			onceTcpDataSize = mtu - ipTcpHdrSize;

		sendPacket->buf_.size_ = ethernetSize + ipTcpHdrSize + onceTcpDataSize;
		ipHdr->tlen_ = htons(ipTcpHdrSize + onceTcpDataSize);
		tcpHdr->sum_ = htons(GTcpHdr::calcChecksum(ipHdr, tcpHdr));
		ipHdr->sum_ = htons(GIpHdr::calcChecksum(ipHdr));

		GPacket::Result res = write(sendPacket->buf_);
		if (res != GPacket::Ok) {
			qWarning() << QString("write(packet->buf_) return %1 len=%2").arg(int(res)).arg(sendPacket->buf_.size_);
			result = res;
		}
		tcpHdr->seq_ = htonl(tcpHdr->seq() + onceTcpDataSize); // next seq
		tcpDataSize -= onceTcpDataSize;
		memcpy(tcpDataData, tcpDataData + onceTcpDataSize, tcpDataSize); // next data
		if (tcpDataSize > 0)
			QThread::usleep(usleepTime);
	}

	return result;
}
