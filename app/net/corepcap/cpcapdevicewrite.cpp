#include "cpcapdevicewrite.h"

bool CPcapDeviceWrite::doOpen() {
	return CPcap::openDeviceForWrite(devName_);
}

bool CPcapDeviceWrite::doClose() {
	return CPcap::doClose();
}
