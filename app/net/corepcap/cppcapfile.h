#pragma once

#include "cppcap.h"

struct LPcapFile : LPcap {
	std::string fileName_{""};
	std::string filter_{""};

public:
	LPcapFile() {}
	~LPcapFile() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;
};
