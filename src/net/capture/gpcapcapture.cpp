#include "gpcapcapture.h"

// ----------------------------------------------------------------------------
// GPcapCapture
// ----------------------------------------------------------------------------
bool GPcapCapture::doOpen() {
	if (!enabled_) return true;

	int dataLink = pcap_datalink(pcap_);
	dlt_ = GPacket::intToDlt(dataLink);

	if (filter_ != "") {
		if (!pcapProcessFilter(nullptr))
			return false;
	}

	return GCapture::doOpen();
}

bool GPcapCapture::doClose() {
	if (!enabled_) return true;

	GCapture::doClose();

	if (pcap_ != nullptr) {
		pcap_close(pcap_);
		pcap_ = nullptr;
	}

	return true;
}

GPacket::Result GPcapCapture::read(GPacket* packet) {
	packet->clear();
	// qDebug() << "bef pcap_next_ex"; // gilgil temp 2017.11.25
	pcap_pkthdr* pktHdr;
	int i = pcap_next_ex(pcap_, &pktHdr, const_cast<const u_char**>(&(packet->buf_.data_)));
	// qDebug() << "aft pcap_next_ex return " << i << "state is" << int(state_); // gilgil temp 2017.11.25
	if (state_ != Opened) return GPacket::Fail; // may be pcap_close called
	GPacket::Result res;
	switch (i) {
		case PCAP_ERROR: { // if an error occurred
			char* e = pcap_geterr(pcap_);
			if (e != nullptr && strlen(e) > 0) {
				QString msg = QString("pcap_next_ex return -1 error=%1").arg(e);
				SET_ERR(GErr::ReadFailed, msg);
			}
			res = GPacket::Eof;
			break;
		}
		case PCAP_ERROR_BREAK: { // if EOF was reached reading from an offline capture
			char* e = pcap_geterr(pcap_);
			if (e != nullptr && strlen(e) > 0) {
				QString msg = QString("pcap_next_ex return -2 error=%1").arg(e);
				SET_ERR(GErr::ReadFailed, msg);
			}
			res = GPacket::Eof;
			break;

		}
		case 0 : // if a timeout occured
			res = GPacket::None;
			break;
		default: // packet captured
			packet->ts_ = pktHdr->ts;
			packet->buf_.size_ = pktHdr->caplen;
			if (autoParse_) packet->parse();
			res = GPacket::Ok;
			break;
	}
	return res;
}

GPacket::Result GPcapCapture::write(GBuf buf) {
	Q_ASSERT(pcap_ != nullptr);
	int i = pcap_sendpacket(pcap_, buf.data_, int(buf.size_));
	if (i == 0) return GPacket::Ok;
	char* e = pcap_geterr(pcap_);
	if (e == nullptr) e = pchar("unknown");
	QString msg = QString("pcap_sendpacket return %1(%2) length=%3").arg(i).arg(e).arg(buf.size_);
	SET_ERR(GErr::Fail, msg);
	return GPacket::Fail;
}

GPacket::Result GPcapCapture::write(GPacket* packet) {
	GPacket::Result res;
	if (mtu_ != 0 && packet->ipHdr_ != nullptr && packet->ipHdr_->len() > uint16_t(mtu_) && packet->tcpHdr_ != nullptr)
		res = writeMtuSplit(packet, mtu_, GPacket::Eth);
	else
		res = write(packet->buf_);
	return res;
}

GPacket::Result GPcapCapture::relay(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NotSupported, "not supported");
	return GPacket::Fail;
}

bool GPcapCapture::pcapProcessFilter(pcap_if_t* dev) {
	u_int uNetMask;
	bpf_program code;

	if (dev != nullptr && dev->addresses != nullptr && dev->addresses->netmask != nullptr)
		uNetMask = (reinterpret_cast<struct sockaddr_in*>(dev->addresses->netmask))->sin_addr.s_addr;
	else
		uNetMask = 0xFFFFFFFF;
	if (pcap_compile(pcap_, &code, qPrintable(filter_), 1, uNetMask) < 0) {
		SET_ERR(GErr::Unknown, QString("error in pcap_compile %1 - %2").arg(pcap_geterr(pcap_)).arg(filter_));
		return false;
	}
	if (pcap_setfilter(pcap_, &code) < 0) {
		SET_ERR(GErr::Unknown, QString("error in pcap_setfilter(%1)").arg(pcap_geterr(pcap_)));
		return false;
	}
	return true;
}
