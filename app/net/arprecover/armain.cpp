#include <csignal>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "gtrace.h"

#include "arprecover.h"

ArpRecover recover;

struct Param {
	bool parse(int argc, char* argv[]) {
		if (argc < 10) {
			usage();
			return false;
		}

		int i = 1;
		if (strcmp(argv[i], "-i") == 0) {
			recover.interval_ = std::stoi(argv[i + 1]);
			i += 2;
		}
		recover.intfName_ = argv[i++];
		recover.network_.gateway_ = Ip(argv[i++]);
		recover.network_.mask_ = Ip(argv[i++]);
		recover.network_.ip_ = Ip(argv[i++]);
		recover.myMac_ = Mac(argv[i++]);

		if ((argc - i) % 4 != 0)
			return false;

		while (i < argc) {
			Flow flow;
			flow.senderIp_ = Ip(argv[i++]);
			flow.senderMac_ = Mac(argv[i++]);
			flow.targetIp_ = Ip(argv[i++]);
			flow.targetMac_ = Mac(argv[i++]);
			IpFlowKey key(flow.senderIp_, flow.targetIp_);
			recover.flowMap_[key] = flow;
		}
		return true;
	}

	void usage() {
		std::cerr << "syntax: arprecover [-i <interval>] <interface> gwip mask myip mymac sip smac tip tmac[sip2 smac2 tip2 tmac2...]\n";
		std::cerr << "sample: arprecover -i 60 wlan0 10.1.1.1 255.255.255.0 10.1.1.3 00:33:33:33:33:33:33 10.1.1.2 00:22:22:22:22:22 10.1.1.1 00:11:11:11:11:11\n";
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
	GTRACE("bef server.stop()");
	recover.close();
	GTRACE("aft server.stop()");
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

int main(int argc, char* argv[]) {
	gtrace_default("127.0.0.1", 8908, false, "arprecover.log");

	prepareSignal();

	if (!param.parse(argc, argv)) return -1;

	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
	getcwd(wd, BUFSIZ);
	const char* version =
#include "../../../version.txt"
	;
	GTRACE("arprecover %s started login=%s argv[0]=%s getcwd=%s %s %s", version, getlogin(), argv[0], wd, __DATE__, __TIME__);

	if (!recover.open()) return -1;
	recover.exec();
	recover.close();

	GTRACE("arprecover terminated successfully");
}
