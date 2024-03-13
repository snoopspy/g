#include <cstring>
#include <iostream>
#include <unistd.h>

#include "gtrace.h"
#include "gaux.h"

#include "cookieedit.h"

using namespace std;

struct Param {
	string profiles_;
	string host_;
	string cookies_;

	bool parse(int argc, char* argv[]) {
		if (argc != 4) {
			usage();
			return false;
		}

		profiles_ = argv[1];
		host_ = argv[2];
		cookies_ = argv[3];
		return true;
	}

	void usage() {
		cerr << "firefox cookie edit version " << GAux::getVersion() << endl;
		cerr << endl;
		cerr << "syntax: ffce [profiles.ini dir] [host] [cookies]\n";
		cerr << "sample: ffce ~/.mozilla/firefox .snoopspy.com \"PHPSESSID=11223344; sso=aabbccdd\"\n";
	}
} param;

#ifdef __linux__
void signalHandler(int signo) {
	const char* signal = GAux::getSignalStr(signo);
	char* msg = strsignal(signo);
	GTRACE("signo=%s(%d) %s", signal, signo, msg);
	if (signo == SIGSEGV)
		exit(-1);
}
#endif // __linux__

int main(int argc, char* argv[]) {
	gtrace_default("127.0.0.1", 8908, true, "ffce.log");

	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
#ifdef __linux__
	if (getcwd(wd, BUFSIZ) == nullptr)
		GTRACE("getcwd return null");
	int res = chdir(GAux::getDir(argv[0]).data());
	if (res != 0)
		GTRACE("chdir return %d", res);
	GTRACE("ffce %s login=%s dir=%s %s %s", GAux::getVersion(), getlogin(), wd, __DATE__, __TIME__);
#endif // __linux__
#ifdef WIN32
	GetCurrentDirectoryA(BUFSIZ, wd);
	GTRACE("arprecover %s started dir=%s %s %s", GAux::getVersion(), wd, __DATE__, __TIME__);
#endif // WIN32

	// ----- gilgil temp 2024.03.11 -----
	/*
	string arguments;
	for (int i = 0; i < argc; i++)
		arguments += argv[i] + string(" ");
	GTRACE("argv=%s", arguments.data());
	*/
	// ----------------------------------

	if (!param.parse(argc, argv)) return -1;

	string fileName = CookieEdit::findSqliteFileName(param.profiles_);
	GTRACE("fiieName=%s", fileName.data());

	CookieEdit ce;
	if (!ce.open(fileName)) {
		GTRACE("ce.open(%s) return false", fileName.data());
		return EXIT_FAILURE;
	}

	if (!ce.insert(param.host_, param.cookies_)) {
		GTRACE("ce.insert(%s %s) return false", param.host_.data(), param.cookies_.data());
		ce.close();
		return EXIT_FAILURE;
	}

	ce.close();

	GTRACE("ffce terminated successfully");
}
