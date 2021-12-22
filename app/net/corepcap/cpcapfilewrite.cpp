#include "cpcapfilewrite.h"

bool CPcapFileWrite::doOpen() {
	if (fileName_ == "") {
		GTRACE("file name is not specified");
		return false;
	}

	pcap_ = pcap_open_dead(dataLink_, 0);
	if (pcap_ == nullptr) {
		GTRACE("pcap_open_dead(%d) return null", dataLink_);
		return false;
	}

	if (fileName_ == "-")
		pcap_dumper_ = pcap_dump_fopen(pcap_, stdout);
	else
		pcap_dumper_ = pcap_dump_open(pcap_, fileName_.c_str());
	if (pcap_dumper_ == nullptr) {
		GTRACE("pcap_dump_fopen return null");
		return false;
	}
	pcap_dump_flush(pcap_dumper_);

	return true;
}

bool CPcapFileWrite::doClose() {
	if (pcap_dumper_ != nullptr) {
		pcap_dump_close(pcap_dumper_);
		pcap_dumper_ = nullptr;
	}
	return CPcap::doClose();
}

CPacket::Result CPcapFileWrite::read(CPacket* packet) {
	(void)packet;
	GTRACE("not supported");
	return CPacket::Fail;
}

CPacket::Result CPcapFileWrite::write(CPacket* packet) {
	pcap_pkthdr pkthdr;
	pkthdr.ts = packet->ts_;
	pkthdr.caplen = pkthdr.len = bpf_u_int32(packet->len_);
	pcap_dump(reinterpret_cast<u_char*>(pcap_dumper_), &pkthdr, packet->buf_);
	pcap_dump_flush(pcap_dumper_);
	return CPacket::Ok;
}
