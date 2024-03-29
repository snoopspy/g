#include "cppcap.h"

bool CPcap::doOpen() {
	GTRACE("virtual function call");
	return false;
}

bool CPcap::doClose() {
	if (pcap_ != nullptr) {
		pcap_close(pcap_);
		pcap_ = nullptr;
	}
	return true;
}

CPacket::Result CPcap::read(CPacket* packet) {
	packet->clear();
	pcap_pkthdr* pkthdr;

	int i = pcap_next_ex(pcap_, &pkthdr, const_cast<const u_char**>(&(packet->buf_)));
	if (state_ != Opened) return CPacket::Fail; // may be pcap_close called
	CPacket::Result res;
	switch (i) {
		case PCAP_ERROR_BREAK: // if EOF was reached reading from an offline capture
		case PCAP_ERROR: { // if an error occurred
			char* e = pcap_geterr(pcap_);
			if (e == nullptr || strlen(e) == 0)
				e = const_cast<char*>("unknown");
			GTRACE("pcap_next_ex return %d error=%s", i, e);
			res = CPacket::Eof;
			break;
		}
		case 0 : // if a timeout occured
			res = CPacket::Timeout;
			break;
		default: // packet captured
			packet->ts_ = pkthdr->ts;
			packet->len_ = size_t(pkthdr->caplen);
			res = CPacket::Ok;
			break;
	}
	return res;
}

CPacket::Result CPcap::write(CPacket* packet) {
	int i = pcap_sendpacket(pcap_, packet->buf_, int(packet->len_));
	if (i == 0) return CPacket::Ok;
	char* e = pcap_geterr(pcap_);
	if (e == nullptr) e = const_cast<char*>("unknown");
	GTRACE("pcap_sendpacket return %d(%s) length=%zu", i, e, packet->len_);
	return CPacket::Fail;
}

bool CPcap::openDevice(std::string devName, int snapLen, int promisc, int readTimeout, std::string filter) {
	if (devName == "") {
		GTRACE("device is not specified");
		return false;
	}

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_live(devName.c_str(), snapLen, promisc, readTimeout, errBuf);
	if (pcap_ == nullptr) {
		GTRACE("pcap_open_live return null - %s", errBuf);
		return false;
	}

	if (filter != "" && !processFilter(filter, nullptr)) return false;

	return true;
}

bool CPcap::openFile(std::string fileName, std::string filter){
	if (fileName == "") {
		GTRACE("file name is not specified");
		return false;
	}

	char errBuf[PCAP_ERRBUF_SIZE];
	if (fileName == "-")
		pcap_ = pcap_fopen_offline(stdin, errBuf);
	else
		pcap_ = pcap_open_offline(fileName.c_str(), errBuf);
	if (pcap_ == nullptr) {
		GTRACE("pcap_open_offline return null -%s", errBuf);
		return false;
	}

	if (filter != "" && !processFilter(filter, nullptr)) return false;

	return true;
}

bool CPcap::openDeviceForWrite(std::string devName){
	if (devName == "") {
		GTRACE("device is not specified");
		return false;
	}

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_live(devName.c_str(), 0, 0, 0, errBuf);
	if (pcap_ == nullptr) {
		GTRACE("pcap_open_live return null - %s", errBuf);
		return false;
	}

	return true;
}

bool CPcap::processFilter(std::string filter, pcap_if_t* dev) {
	u_int uNetMask;
	bpf_program code;

	if (dev != nullptr && dev->addresses != nullptr && dev->addresses->netmask != nullptr)
		uNetMask = (reinterpret_cast<struct sockaddr_in*>(dev->addresses->netmask))->sin_addr.s_addr;
	else
		uNetMask = 0xFFFFFFFF;
	if (pcap_compile(pcap_, &code, filter.c_str(), 1, uNetMask) < 0) {
		GTRACE("error in pcap_compile(%s)", filter.c_str());
		return false;
	}
	if (pcap_setfilter(pcap_, &code) < 0) {
		GTRACE("error in pcap_setfilter(%s)", pcap_geterr(pcap_));
		return false;
	}
	return true;
}

int CPcap::datalink() {
	if (pcap_ == nullptr) return DLT_NULL;
	return pcap_datalink(pcap_);
}
