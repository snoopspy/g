#pragma once

#include "cppcap.h"

struct LPcapFileWrite : LPcap {
	std::string fileName_{""};
	int dataLink_{DLT_EN10MB};

public:
	LPcapFileWrite() {}
	~LPcapFileWrite() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	LPacket::Result read(LPacket* packet) override;
	LPacket::Result write(LPacket* packet) override;

protected:
	pcap_dumper_t* pcap_dumper_{nullptr};
};
