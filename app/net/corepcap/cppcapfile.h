#pragma once

#include "cppcap.h"

struct CPcapFile : CPcap {
	std::string fileName_{""};
	std::string filter_{""};

public:
	CPcapFile() {}
	~CPcapFile() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;
};
