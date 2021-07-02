#include "gpcappipenexmon.h"

// ----------------------------------------------------------------------------
// GPcapPipeNexmon
// ----------------------------------------------------------------------------
GPcapPipeNexmon::GPcapPipeNexmon(QObject* parent) : GPcapPipe(parent) {
	QString path = "/data/data/com.snoopspy/files";
	command_ = QString("adb exec-out su -c \"export LD_LIBRARY_PATH=%1/../lib; export LD_PRELOAD=libfakeioctl.so; %2/corepcap dev wlan0 -f '' file -\"").arg(path, path);
}

GPcapPipeNexmon::~GPcapPipeNexmon() {
}
