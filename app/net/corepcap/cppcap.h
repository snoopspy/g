#pragma once

#include "cpobj.h"
#include <pcap.h>

struct LPcap : LObj {
public:
	LPcap() {}
	~LPcap() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	LPacket::Result read(LPacket* packet) override;
	LPacket::Result write(LPacket* packet) override;

protected:
	pcap_t* pcap_{nullptr};

	bool openDevice(std::string devName, int snapLen, int promisc, int readTimeout, std::string filter);
	bool openFile(std::string fileName, std::string filter);
	bool openDeviceForWrite(std::string devName);
	bool processFilter(std::string filter, pcap_if_t* dev);

public:
	int datalink();
};
