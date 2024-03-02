#include <iostream>
#include <unistd.h>

#include "gtrace.h"
#include "gaux.h"
#include "gdemonserver.h"

using namespace std;

GDemonServer server;

struct Param {
	uint16_t port_{GDemon::DefaultPort};

	bool parse(int argc, char* argv[]) {
		if (argc != 2) {
			usage();
			return false;
		}
		port_ = stoi(argv[1]);
		return true;
	}

	void usage() {
		cerr << "ssdemon version "
#include "../../../version.txt"
			 << endl;
		cerr << "" << endl;
		cerr << "syntax: ssdemon <port>\n";
		cerr << "sample: ssdemon 8908\n";
	}
} param;

void signalHandler(int signo) {
	const char* signal = GAux::getSignalStr(signo);
	char* msg = strsignal(signo);
	GTRACE("signo=%s(%d) %s", signal, signo, msg);
	if (signo == SIGSEGV)
		exit(-1);
	GTRACE("bef server.stop()");
	server.stop();
	GTRACE("aft server.stop()");
}

void runServer() {
	if (!server.start(param.port_)) {
		return;
	}
	server.exec();
	server.stop();
	server.wait();
}

int main(int argc, char* argv[]) {
	gtrace_default("127.0.0.1", 8908, false, "ssdemon.log");

	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
	if (getcwd(wd, BUFSIZ) == nullptr)
		GTRACE("getcwd return null");
	int res = chdir(GAux::getDir(argv[0]).data());
	if (res != 0)
		GTRACE("chdir return %d", res);
	GTRACE("ssdemon %s started login=%s dir=%s %s %s", GAux::getVersion(), getlogin(), wd, __DATE__, __TIME__);

	string arguments;
	for (int i = 0; i < argc; i++)
		arguments += argv[i] + string(" ");
	GTRACE("argv=%s", arguments.data());

	GAux::prepareSignal(signalHandler);

	if (!param.parse(argc, argv)) return -1;

	runServer();

	GTRACE("ssdemon terminated successfully");
}
