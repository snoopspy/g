#include <cstring>
#include <iostream>
#include "gdemonserver.h"
#include "gtrace.h"

using namespace std;

void usage() {
	cerr << "syntax: ssdemon [<port>]\n";
	cerr << "sample: ssdemon 8908\n";
}

struct Param {
	uint16_t port_{GDemon::DefaultPort};

	bool parse(int argc, char* argv[]) {
		if (argc >= 2)
			port_ = stoi(argv[1]);
		return true;
	}
} param;

void runServer() {
	GDemonServer server;
	if (!server.start(param.port_)) {
		return;
	}
	server.exec();
	server.stop();
	server.wait();
}

#include <csignal>
void signalHandler(int signo) {
	std::string signal = "unknown";
	switch (signo) {
		case SIGINT: signal = "SIGINT"; break;
		case SIGILL: signal = "SIGILL"; break;
		case SIGABRT: signal = "SIGABRT"; break;
		case SIGFPE: signal = "SIGFPE"; break;
		case SIGSEGV: signal = "SIGSEGV"; break;
		case SIGTERM: signal = "SIGTERM"; break;
		case SIGHUP: signal = "SIGHUP"; break;
		case SIGQUIT: signal = "SIGQUIT"; break;
		case SIGTRAP: signal = "SIGTRAP"; break;
		case SIGKILL: signal = "SIGKILL"; break;
		case SIGBUS: signal = "SIGBUS"; break;
		case SIGSYS: signal = "SIGSYS"; break;
		case SIGPIPE: signal = "SIGPIPE"; break;
		case SIGALRM: signal = "SIGALRM"; break;
	}
	std::string msg = strsignal(signo);
	GTRACE("signalHandler signo=%s(%d) %s", signal.data(), signo, msg.data());
	exit(0);
}

int main(int argc, char* argv[]) {
	gtrace_default("127.0.0.1", 8908, true, nullptr);
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}

	std::signal(SIGINT, signalHandler);
	std::signal(SIGILL, signalHandler);
	std::signal(SIGABRT, signalHandler);
	std::signal(SIGFPE, signalHandler);
	std::signal(SIGSEGV, signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGHUP, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	std::signal(SIGTRAP, signalHandler);
	std::signal(SIGKILL, signalHandler);
	std::signal(SIGBUS, signalHandler);
	std::signal(SIGSYS, signalHandler);
	std::signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE which can be signaled when TCP socket operation on linux
	std::signal(SIGALRM, signalHandler);

	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
	getcwd(wd, BUFSIZ);
	GTRACE("ssdemon started argv[0]=%s getcwd=%s", argv[0], wd);
	runServer();
	GTRACE("ssdemon terminated");
}
