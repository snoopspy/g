#pragma once

#include <list>
#include "cppcap.h"

struct LCorePcap : LObj {
	LPcap* input_{nullptr};
	std::list<LObj*> outputs_;
	std::string error_;

	LCorePcap() {}
	~LCorePcap() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	static void usage();
	bool parse(int argc, char* argv[]);
	void run();
};
