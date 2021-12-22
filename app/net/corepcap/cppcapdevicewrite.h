#pragma once

#include "cppcap.h"

struct CPcapDeviceWrite : CPcap {
	std::string devName_{""};

public:
	CPcapDeviceWrite() {}
	~CPcapDeviceWrite() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;
};
