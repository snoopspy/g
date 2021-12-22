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

		GTRACE("interval=%d", recover.interval_);
		GTRACE("intfName=%s", recover.intfName_.data());
		GTRACE("gateway=%s", std::string(recover.network_.gateway_).data());
		GTRACE("mask=%s", std::string(recover.network_.mask_).data());
		GTRACE("ip=%s", std::string(recover.network_.ip_).data());
		GTRACE("mac=%s", std::string(recover.myMac_).data());

		while (i < argc) {
			Flow flow;
			flow.senderIp_ = Ip(argv[i++]);
			flow.senderMac_ = Mac(argv[i++]);
			flow.targetIp_ = Ip(argv[i++]);
			flow.targetMac_ = Mac(argv[i++]);
			GTRACE("sip=%s smac=%s tip=%s tmac=%s",
				std::string(flow.senderIp_).data(),
				std::string(flow.senderMac_).data(),
				std::string(flow.targetIp_).data(),
				std::string(flow.targetMac_).data());
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

#ifdef __linux__
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
#endif // __linux__

std::string getDir(std::string argv) {
	ssize_t i = argv.length() - 1;
	while (i >= 0) {
		char& ch = argv.at(i);
		if (ch == '/' || ch == '\\') {
			std::string res = argv.substr(0, i + 1);
			return res;
		}
		i--;
	}
	return "/";
}

int main(int argc, char* argv[]) {
	gtrace_default("127.0.0.1", 8908, false, "arprecover.log");

	const char* version =
#include "../../../version.txt"
	;
	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
#ifdef __linux__
	getcwd(wd, BUFSIZ);
	chdir(getDir(argv[0]).data());
	GTRACE("arprecover %s started login=%s dir=%s %s %s", version, getlogin(), wd, __DATE__, __TIME__);
#endif // __linux__
#ifdef WIN32
	GetCurrentDirectoryA(BUFSIZ, wd);
	GTRACE("arprecover %s started dir=%s %s %s", version, wd, __DATE__, __TIME__);
#endif // WIN32

	std::string arguments;
	for (int i = 0; i < argc; i++)
		arguments += argv[i] + std::string(" ");
	GTRACE("argv=%s", arguments.data());

#ifdef __linux__
	prepareSignal();
#endif // __linux__

	if (!param.parse(argc, argv)) return -1;

	if (!recover.open()) return -1;
	recover.exec();
	recover.close();

	GTRACE("arprecover terminated successfully");
}
