#include <csignal>
#include <unistd.h>
#include "cpcorepcap.h"

CCorePcap cp;

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
	GTRACE("bef cp.close()");
	cp.close();
	GTRACE("aft cp.close()");
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
	gtrace_default("127.0.0.1", 8908, false, "corepcap.log");

	const char* version =
#include "../../../version.txt"
	;
	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
	getcwd(wd, BUFSIZ);
	chdir(getDir(argv[0]).data());
	GTRACE("corepcap %s started login=%s dir=%s %s %s", version, getlogin(), wd, __DATE__, __TIME__);

	std::string arguments;
	for (int i = 0; i < argc; i++)
		arguments += argv[i] + std::string(" ");
	GTRACE("argv=%s", arguments.data());

	prepareSignal();

	if (!cp.parse(argc, argv)) {
		GTRACE("parse fail(%s)", cp.error_.c_str());
		return -1;
	}

	if (!cp.open())
		return -1;
	cp.run();
	if (cp.active())
		cp.close();

	GTRACE("corepcap terminated successfully");
}
