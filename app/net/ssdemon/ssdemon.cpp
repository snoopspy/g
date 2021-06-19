#include <csignal>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "gdemonserver.h"
#include "gtrace.h"

using namespace std;

GDemonServer server;

struct Param {
	uint16_t port_{GDemon::DefaultPort};

	bool parse(int argc, char* argv[]) {
		if (argc == 2)
			port_ = stoi(argv[1]);
		if (argc > 2) {
			usage();
			return false;
		}
		return true;
	}

	void usage() {
		cerr << "syntax: ssdemon [<port>]\n";
		cerr << "sample: ssdemon 8908\n";
	}
} param;

void signalHandler(int signo) {
	const char* signal = "unknown";
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
		case SIGURG: signal = "SIGURG"; break;
		case SIGSTOP: signal = "SIGSTOP"; break;
		case SIGTSTP: signal = "SIGTSTP"; break;
		case SIGCONT: signal = "SIGCONT"; break;
		case SIGCHLD: signal = "SIGCHLD"; break;
		case SIGTTIN: signal = "SIGTTIN"; break;
		case SIGTTOU: signal = "SIGTTOU"; break;
		case SIGPOLL: signal = "SIGPOLL"; break;
		case SIGXCPU: signal = "SIGXCPU"; break;
		case SIGXFSZ: signal = "SIGXFSZ"; break;
		case SIGVTALRM: signal = "SIGVTALRM"; break;
		case SIGPROF: signal = "SIGPROF"; break;
		case SIGUSR1: signal = "SIGUSR1"; break;
		case SIGUSR2: signal = "SIGUSR2"; break;
	}
	char* msg = strsignal(signo);
	GTRACE("signo=%s(%d) %s", signal, signo, msg);
	if (signo == SIGSEGV)
		exit(-1);
	server.stop();
}

void prepareSignal() {
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

	prepareSignal();

	if (!param.parse(argc, argv)) {
		return -1;
	}

	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
	getcwd(wd, BUFSIZ);
	const char* version =
#include "../../../version.txt"
	;
	GTRACE("ssdemon %s started login=%s argv[0]=%s getcwd=%s %s %s", version, getlogin(), argv[0], wd, __DATE__, __TIME__);

	runServer();

	GTRACE("ssdemon terminated successfully");
}
