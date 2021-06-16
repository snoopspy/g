#include "cppcapdevicewrite.h"

bool LPcapDeviceWrite::doOpen() {
	return LPcap::openDeviceForWrite(devName_);
}

bool LPcapDeviceWrite::doClose() {
	return LPcap::doClose();
}
