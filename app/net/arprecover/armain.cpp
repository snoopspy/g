#include <iostream>
#include <unistd.h>

#include "gtrace.h"
#include "gaux.h"
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
	const char* signal = GAux::getSignalStr(signo);
	char* msg = strsignal(signo);
	GTRACE("signo=%s(%d) %s", signal, signo, msg);
	if (signo == SIGSEGV)
		exit(-1);
	GTRACE("bef server.stop()");
	recover.close();
	GTRACE("aft server.stop()");
}
#endif // __linux__

int main(int argc, char* argv[]) {
	gtrace_default("127.0.0.1", 8908, false, "arprecover.log");

	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
#ifdef __linux__
	getcwd(wd, BUFSIZ);
	chdir(GAux::getDir(argv[0]).data());
	GTRACE("arprecover %s started login=%s dir=%s %s %s", GAux::getVersion(), getlogin(), wd, __DATE__, __TIME__);
#endif // __linux__
#ifdef WIN32
	GetCurrentDirectoryA(BUFSIZ, wd);
	GTRACE("arprecover %s started dir=%s %s %s", GAux::getVersion(), wd, __DATE__, __TIME__);
#endif // WIN32

	std::string arguments;
	for (int i = 0; i < argc; i++)
		arguments += argv[i] + std::string(" ");
	GTRACE("argv=%s", arguments.data());

#ifdef __linux__
	GAux::prepareSignal(signalHandler);
#endif // __linux__

	if (!param.parse(argc, argv)) return -1;

	if (!recover.open()) return -1;
	recover.exec();
	recover.close();

	GTRACE("arprecover terminated successfully");
}
