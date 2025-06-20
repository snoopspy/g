#include "gpcappipenexmon.h"

// ----------------------------------------------------------------------------
// GPcapPipeNexmon
// ----------------------------------------------------------------------------
GPcapPipeNexmon::GPcapPipeNexmon(QObject* parent) : GPcapPipe(parent) {
	QString path = "/data/data/com.snoopspy/files";
	command_ = QString("adb exec-out su -c \"export LD_PRELOAD=libnexmon.so; %1/corepcap dev wlan0 file -\"").arg(path);
}

GPcapPipeNexmon::~GPcapPipeNexmon() {
}
