#include "gpcappipenexmon.h"

// ----------------------------------------------------------------------------
// GPcapPipeNexmon
// ----------------------------------------------------------------------------
GPcapPipeNexmon::GPcapPipeNexmon(QObject* parent) : GPcapPipe(parent) {
	QString path = "/data/data/com.snoopspy/files";
	command_ = QString("adb exec-out su -c \"cd %1; export LD_LIBRARY_PATH=%2/../lib; export LD_PRELOAD=libfakeioctl.so; ./corepcap dev wlan0 -f '' file -\"").arg(path, path);
}

GPcapPipeNexmon::~GPcapPipeNexmon() {
}
