#include <iostream>
#include <sstream>
#include <unistd.h>

#include "gtrace.h"
#include "gaux.h"
#include "giw.h"

struct Param {
	std::string intfName_;

	enum Command {
		GetChannel,
		SetChannel,
		List,
		Hop
	} command_{GetChannel};

	int setChannelNo_;
	std::list<int>channelHopList_;

	bool parse(int argc, char** argv) {
		if (argc <= 1)
			return false;
		intfName_ = argv[1];

		if (argc <= 2)
			return true;

		std::string s = argv[2];
		if (std::isdigit(s[0])) {
			command_ = SetChannel;
			setChannelNo_ = std::stoi(s);
			return true;
		}

		if (s == "list") {
			command_ = List;
			return true;
		} else if (s != "hop") {
			return false;
		}

		if (argc <= 3)
			return true;

		s = argv[3];
		std::stringstream ss(s);

		for (int i; ss >> i;) {
			channelHopList_.push_back(i);
			if (ss.peek() == ',')
				ss.ignore();
		}
		return true;
	}

	static void usage() {
		std::cerr << "snoopspy channel " << GAux::getVersion() << "\n";
		std::cerr << "  ssch <interface> [channel no]\n";
		std::cerr << "  ssch <interface> list\n";
		std::cerr << "  ssch <interface> hop [channel list]\n";
		std::cerr << "examples:";
		std::cerr << "  ssch wlan0\n";
		std::cerr << "  ssch wlan0 5\n";
		std::cerr << "  ssch wlan0 list\n";
		std::cerr << "  ssch wlan0 hop\n";
		std::cerr << "  ssch wlan0 hop 1,2,3,4,5,6,7,8,9,10\n";
	}
} param;

void signalHandler(int signo) {
	const char* signal = GAux::getSignalStr(signo);
	char* msg = strsignal(signo);
	GTRACE("signo=%s(%d) %s", signal, signo, msg);
	if (signo == SIGSEGV)
		exit(-1);
	GTRACE("bef server.stop()");
	// recover.close(); // gilgil temp
	GTRACE("aft server.stop()");
}

int main(int argc, char* argv[]) {
	gtrace_default("127.0.0.1", 8908, false, "ssch.log");

	char wd[BUFSIZ];
	memset(wd, 0, BUFSIZ);
	getcwd(wd, BUFSIZ);
	chdir(GAux::getDir(argv[0]).data());
	GTRACE("ssch %s started login=%s dir=%s %s %s", GAux::getVersion(), getlogin(), wd, __DATE__, __TIME__);

	if (!param.parse(argc, argv)) {
		Param::usage();
		return -1;
	}

	switch (param.command_) {
		case Param::GetChannel:
			GTRACE("GetChannel");
			break;
		case Param::SetChannel:
			GTRACE("SetChannel");
			break;
		case Param::List:
			GTRACE("List");
			break;
		case Param::Hop:
			GTRACE("Hop");
			break;
	}
}
