#pragma once

#include "cppcap.h"

struct LPcapDevice : LPcap {
	std::string devName_{""};
	int snapLen_{32768}; // 32768 bytes
	int promisc_{1}; // PCAP_OPENFLAG_PROMISCUOUS
	int readTimeout_{-1}; // -1 msec
	int waitTimeout_{1}; // 1 msec
	int adjustFrameSize_{0};
	std::string filter_{""};

public:
	LPcapDevice() {}
	~LPcapDevice() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	LPacket::Result read(LPacket* packet) override;
};
