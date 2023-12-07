#include "gbpfilter.h"

// ----------------------------------------------------------------------------
// GBpFilter
// ----------------------------------------------------------------------------
bool GBpFilter::doOpen() {
	int res;

	int dataLink = GPacket::dltToInt(dlt_);
	pcap_ = pcap_open_dead(dataLink, 1);
	if (pcap_ == nullptr) {
		SET_ERR(GErr::Fail, "error in pcap_open_dead return null");
		return false;
	}

	code_ = static_cast<bpf_program*>(malloc(sizeof(bpf_program)));
	res = pcap_compile(pcap_, code_, qPrintable(filter_), 1, 0xFFFFFFFF);
	if (res < 0) {
		SET_ERR(GErr::Fail, QString("error in pcap_compile %1 - %2").arg(pcap_geterr(pcap_)).arg(filter_));
		return false;
	}

	return true;
}

bool GBpFilter::doClose() {
	if (pcap_ != nullptr) {
		pcap_close(pcap_);
		pcap_ = nullptr;
	}

	if (code_ != nullptr) {
		pcap_freecode(code_);
		free(code_);
		code_ = nullptr;
	}

	return true;
}

bool GBpFilter::check(GBuf buf) {
	Q_ASSERT(code_ != nullptr);
	u_int len = u_int(buf.size_);
	u_int res = bpf_filter(code_->bf_insns, const_cast<u_char*>(buf.data_), len, len);
	return res > 0;
}

bool GBpFilter::check(GPacket* packet) {
	bool res = check(packet->buf_);
	if (res)
		emit filtered(packet);
	else
		emit notFiltered(packet);
	return res;
}
