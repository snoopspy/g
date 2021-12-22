#pragma once

#include "cobj.h"
#include <pcap.h>

struct CPcap : CObj {
public:
	CPcap() {}
	~CPcap() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	CPacket::Result read(CPacket* packet) override;
	CPacket::Result write(CPacket* packet) override;

protected:
	pcap_t* pcap_{nullptr};

	bool openDevice(std::string devName, int snapLen, int promisc, int readTimeout, std::string filter);
	bool openFile(std::string fileName, std::string filter);
	bool openDeviceForWrite(std::string devName);
	bool processFilter(std::string filter, pcap_if_t* dev);

public:
	int datalink();
};
