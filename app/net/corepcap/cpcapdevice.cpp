#include "cpcapdevice.h"
#include <thread>

bool CPcapDevice::doOpen() {
	return CPcap::openDevice(devName_, snapLen_, promisc_, -1, filter_);
}

bool CPcapDevice::doClose() {
	return CPcap::doClose();
}

CPacket::Result CPcapDevice::read(CPacket* packet) {
	CPacket::Result	res = CPcap::read(packet);
	if (res == CPacket::Ok)
		packet->len_ += adjustFrameSize_;
	if (res == CPacket::Timeout)
		std::this_thread::sleep_for(std::chrono::milliseconds(waitTimeout_));
	return res;
}

#ifdef GTEST
#include <gtest/gtest.h>

TEST(PcapDevice, openCloseTest) {
	CPcapDevice device;
	EXPECT_FALSE(device.open());
	device.close();

	device.devName_ = "unknown-device";
	EXPECT_FALSE(device.open());
	device.close();

	device.devName_ = "wlan0";
	EXPECT_TRUE(device.open());
	device.close();
}

#endif // GTEST
