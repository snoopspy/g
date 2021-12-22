#pragma once

#include "cpcap.h"

struct CPcapFileWrite : CPcap {
	std::string fileName_{""};
	int dataLink_{DLT_EN10MB};

public:
	CPcapFileWrite() {}
	~CPcapFileWrite() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	CPacket::Result read(CPacket* packet) override;
	CPacket::Result write(CPacket* packet) override;

protected:
	pcap_dumper_t* pcap_dumper_{nullptr};
};
