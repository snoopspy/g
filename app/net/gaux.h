// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include <csignal>
#include <string>

struct GAux {
#ifdef __linux__
	static void prepareSignal(__sighandler_t signalHandler);
	static const char* getSignalStr(int signo);
#endif
	static std::string getDir(std::string argv);
	static const char* getVersion();
};
