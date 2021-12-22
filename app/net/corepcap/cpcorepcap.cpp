#include "cpcorepcap.h"
#include "cppcapdevice.h"
#include "cppcapdevicewrite.h"
#include "cppcapfile.h"
#include "cppcapfilewrite.h"

#include <iostream>

using namespace std;

CCorePcap::~CCorePcap() {
	if (input_ != nullptr)
		input_->close();
	for (CObj* output: outputs_)
		output->close();
	if (input_ != nullptr) {
		delete input_;
		input_ = nullptr;
	}
	for (CObj* obj: outputs_)
		delete obj;
}

bool CCorePcap::doOpen() {
	if (!input_->open()) return false;
	GTRACE("input datalink is %d", input_->datalink());
	for (CObj* output: outputs_) {
		CPcapFileWrite* fileWrite = dynamic_cast<CPcapFileWrite*>(output);
		if (fileWrite != nullptr)
			fileWrite->dataLink_ = input_->datalink();
		if (!output->open()) return false;
	}
	return true;
}

bool CCorePcap::doClose() {
	input_->close();
	for (CObj* output: outputs_)
		output->close();
	return true;
}

void CCorePcap::usage() {
	cerr << "corepcap version "
#include "../../../version.txt"
	<< endl;
	cerr << "" << endl;
	cerr << "syntax" << endl;
	cerr << "  corepcap <input> <output>[<output>...]" << endl;
	cerr << "    * Input must be specified just once, meanwhile, output could be multiple instance." << endl;
	cerr << "  " << endl;
	cerr << "  input ::= <schema> <name> [input-options]" << endl;
	cerr << "  " << endl;
	cerr << "  output ::= <schema> <name>" << endl;
	cerr << "  " << endl;
	cerr << "  schema ::= dev | file" << endl;
	cerr << "  " << endl;
	cerr << "  name ::= e.g., eth0, wlan0, test.pcap, -, etc" << endl;
	cerr << "    * \"-\" is stdin(input) or stdout(output)." << endl;
	cerr << "  " << endl;
	cerr << "  input-options ::= "" | -f <filter> | -l <snap len> | -p <promisc> | -rt <read timeout> | -wt <wait timeout> | -afs <adjust frame size> [input-options]" << endl;
	cerr << "    * filter : default(\"\")" << endl;
	cerr << "    * snap len : default(32768 bytes)" << endl;
	cerr << "    * promisc : default(1 PCAP_OPENFLAG_PROMISCUOUS)" << endl;
	cerr << "    * read timeout : default(-1 msec)" << endl;
	cerr << "    * wait timeout : default(1 msec)" << endl;
	cerr << "    * adjust frame size : default(0)" << endl;
	cerr << "" << endl;
	cerr << "example" << endl;
	cerr << "  corepcap dev eth0 file -" << endl;
	cerr << "  corepcap dev wlan0 -f tcp file - file tcp.pcap"<< endl;
	cerr << "  corepcap dev mon0 -f \"wlan[0] != 0x80\" file -"<< endl;
	cerr << "  corepcap dev mon0 -f \"radio[2:2] == 0x2800\" file -"<< endl;
	cerr << "  corepcap file - dev dum0"<< endl;
}

bool CCorePcap::parse(int argc, char* argv[]) {
	if (argc == 1) {
		usage();
		return false;
	}

	int i = 1;
	//
	// input
	//
	if (i == argc) { error_ = "[input] schema not specified"; return false; }
	string schema = argv[i++];

	if (i == argc) { error_ = "[input] name not specified for schema " + schema; return false; }
	string name = argv[i++];

	if (schema == "dev") {
		CPcapDevice* device = new CPcapDevice;
		device->devName_ = name;
		while (true) {
			if (i == argc) break;
			string option = argv[i];
			if (option.at(0) != '-') break;
			i++;
			if (i == argc) { error_ = "[input] value not specified for option " + option; return false; }
			if (option == "-f")
				device->filter_ = argv[i++];
			else if (option == "-l")
				device->snapLen_ = stoi(argv[i++]);
			else if (option == "-p")
				device->promisc_ = stoi(argv[i++]);
			else if (option == "-rt")
				device->readTimeout_ = stoi(argv[i++]);
			else if (option == "-wt")
				device->waitTimeout_ = stoi(argv[i++]);
			else if (option == "-afs")
				device->adjustFrameSize_ = stoi(argv[i++]);
			else { error_ = "[input] invalid option " + option; return false; }
		}
		input_ = device;
	} else if (schema == "file") {
		CPcapFile* file = new CPcapFile;
		file->fileName_ = name;
		while (true) {
			if (i == argc) break;
			string option = argv[i];
			if (option.at(0) != '-') break;
			i++;
			if (i == argc) { error_ = "[output] value not specified for option " + option; return false; }
			if (option == "-filter")
				file->filter_ = argv[i++];
			else { error_ = "[output] invalid option " + option; return false; }
		}
		input_ = file;
	} else { error_ = "[input] invalid schema " + schema; return false; }

	while (i < argc) {
		if (i == argc) { error_ = "[output] schema not specified"; return false; }
		string schema = argv[i++];
		if (i == argc) { error_ = "[output] name not specified for schema " + schema; return false; }
		string name = argv[i++];
		if (schema == "dev") {
			CPcapDeviceWrite* deviceWrite = new CPcapDeviceWrite;
			deviceWrite->devName_ = name;
			outputs_.push_back(deviceWrite);
		} else if (schema == "file") {
			CPcapFileWrite* fileWrite = new CPcapFileWrite;
			fileWrite->fileName_ = name;
			outputs_.push_back(fileWrite);
		} else { error_ = "[output] invalid schema " + schema; return false; }
	}

	if (outputs_.size() == 0) { error_ = "at least ont output must be specified"; return false; }

	return true;
}

void CCorePcap::run() {
	GTRACE("beg");
	while (input_->active()) {
		CPacket packet;
		CPacket::Result res = input_->read(&packet);
		if (res == CPacket::Timeout) continue;
		if (res != CPacket::Ok) break;
		for (CObj* output: outputs_) {
			res = output->write(&packet);
			if (res != CPacket::Ok) break;
		}
	}
	GTRACE("end");
}
