#include "gbase.h"
#include <pcap.h>

int _debug_gilgil{0};
__thread int _thread_gilgil{0};

const char* G::version() {
	return
#ifdef _DEBUG
#include "../../version.txt"
	" Debug Build(" __DATE__ " " __TIME__ ")";
#else // RELEASE
#include "../../version.txt"
	" Release Build(" __DATE__ " " __TIME__ ")";
#endif // _DEBUG
}

const char* G::pcapLibVersion() {
	return pchar(::pcap_lib_version());
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

#include <iostream>
TEST(GBase, versionTest) {
	std::cout << G::version() << std::endl;
	std::cout << G::pcap_lib_version() << std::endl;
}

#endif // GTEST
