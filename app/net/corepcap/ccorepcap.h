#pragma once

#include <list>
#include "cpcap.h"

struct CCorePcap : CObj {
	CPcap* input_{nullptr};
	std::list<CObj*> outputs_;
	std::string error_;

	CCorePcap() {}
	~CCorePcap() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	static void usage();
	bool parse(int argc, char* argv[]);
	void run();
};
