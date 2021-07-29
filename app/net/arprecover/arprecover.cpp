#include "arprecover.h"
#include <chrono>

Flow::Flow(Ip senderIp, Mac senderMac, Ip targetIp, Mac targetMac) {
	senderIp_ = senderIp;
	senderMac_ = senderMac;
	targetIp_ = targetIp;
	targetMac_ = targetMac;
}

void Flow::makePacket(EthArpPacket* packet, Mac myMac) {
	packet->ethHdr_.dmac_ = senderMac_;
	packet->ethHdr_.smac_ = myMac;
	packet->ethHdr_.type_ = htons(EthHdr::Arp);

	packet->arpHdr_.hrd_ = htons(ArpHdr::ETHER);
	packet->arpHdr_.pro_ = htons(EthHdr::Ip4);
	packet->arpHdr_.hln_ = sizeof(Mac);
	packet->arpHdr_.pln_ = sizeof(Ip);
	packet->arpHdr_.op_ = htons(ArpHdr::Reply);
	packet->arpHdr_.smac_ = targetMac_;
	packet->arpHdr_.sip_ = htonl(targetIp_);
	packet->arpHdr_.tmac_ = senderMac_;
	packet->arpHdr_.tip_ = htonl(senderIp_);
}

ArpRecover::ArpRecover() {
}

ArpRecover::~ArpRecover() {
	close();
}

void ArpRecover::_sendThread(ArpRecover* arpRecover) {
	arpRecover->sendThread();
}

void ArpRecover::sendThread() {
	while (active_) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		flowMap_.lock();
		for (FlowMap::iterator it = flowMap_.begin(); it != flowMap_.end(); it++) {
			Flow& flow = it->second;
			flow.makePacket(&flow.recoverPacket_, myMac_);
			int i = pcap_sendpacket(pcap_, (const u_char*)&flow.recoverPacket_, sizeof(flow.recoverPacket_)); // recover
			if (i != 0)
				GTRACE("pcap_sendpacket return %d", i);
		}
		flowMap_.unlock();
	}
}

bool ArpRecover::open() {
	for (FlowMap::iterator it = flowMap_.begin(); it != flowMap_.end(); it++) {
		Flow& flow = it->second;
		flow.makePacket(&flow.recoverPacket_, myMac_);
	}

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_live(intfName_.data(), 32768, 1, -1, errBuf);
	if (pcap_ == nullptr) {
		GTRACE("ope_open_live return null - %s", errBuf);
		return false;
	}

	sendThread_ = new std::thread(ArpRecover::_sendThread, this);

	active_ = true;
	return true;
}

typedef std::chrono::high_resolution_clock::time_point Clock;
typedef std::chrono::high_resolution_clock::duration Diff;
typedef std::chrono::high_resolution_clock Timer;
bool ArpRecover::exec() {
	GTRACE("beg");

	Clock start = Timer::now();
	while (active_) {
		Clock now = Timer::now();
		Diff diff = now - start;
		if (diff > Diff(std::chrono::seconds(interval_)))
			break;

		pcap_pkthdr* pktHdr;
		const u_char* packet;
		int i = pcap_next_ex(pcap_, &pktHdr, &packet);
		switch (i) {
			case PCAP_ERROR_BREAK:
			case PCAP_ERROR: {
				char* e = pcap_geterr(pcap_);
				GTRACE("pcap_next_ex return %d - %s" ,i, e);
				active_ = false;
				break;
			}
			case 0:
				std::this_thread::sleep_for(std::chrono::microseconds(1));
				continue;
		}

		EthHdr* ethHdr = PEthHdr(packet);
		if (ethHdr->smac() == myMac_) continue;
		if (ethHdr->dmac().isBroadcast() || ethHdr->dmac().isMulticast()) continue;

		if (ethHdr->type() != EthHdr::Ip4) continue;

		IpHdr* ipHdr = PIpHdr(packet + sizeof(EthHdr));
		Ip sip = ipHdr->sip();
		Ip dip = ipHdr->dip();
		Ip adjSip = network_.getAdjIp(sip);
		Ip adjDip = network_.getAdjIp(dip);

		IpFlowKey key(adjSip, adjDip);

		flowMap_.lock();
		FlowMap::iterator it = flowMap_.find(key);
		if (it == flowMap_.end()) {
			flowMap_.unlock();
			continue;
		}

		Flow& flow = it->second;
		GTRACE("relay! sip=%s dip=%s", std::string(sip).data(), std::string(dip).data());
		ethHdr->smac_ = myMac_;
		ethHdr->dmac_ = flow.targetMac_;
		i = pcap_sendpacket(pcap_, packet, pktHdr->caplen); // relay
		if (i != 0)
			GTRACE("pcap_sendpacket return %d", i);
		i = pcap_sendpacket(pcap_, (const u_char*)&flow.recoverPacket_, sizeof(flow.recoverPacket_)); // recover
		if (i != 0)
			GTRACE("pcap_sendpacket return %d", i);
		flowMap_.unlock();
	}
	GTRACE("end");
	return true;
}

bool ArpRecover::close() {
	active_ = false;
	if (sendThread_ != nullptr) {
		sendThread_->join();
		delete sendThread_;
		sendThread_ = nullptr;
	}
	return true;
}

