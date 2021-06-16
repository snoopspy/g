#pragma once

#include "cppcap.h"

struct LPcapDeviceWrite : LPcap {
	std::string devName_{""};

public:
	LPcapDeviceWrite() {}
	~LPcapDeviceWrite() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;
};
