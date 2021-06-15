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

#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// GProcess
// ----------------------------------------------------------------------------
struct GProcess {
	static bool execute(std::string command, std::string& error);
	static pid_t start(std::string command, std::string& error);
	static bool stop(pid_t pid, std::string& error);
	static bool startDetached(std::string command, std::string& error);

	static std::vector<std::string> splitString(std::string s, char ch);
	static std::vector<std::string> splitCommand(std::string command);
};

