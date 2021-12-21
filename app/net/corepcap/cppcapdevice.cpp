#include "cppcapdevice.h"
#include <thread>

bool LPcapDevice::doOpen() {
	return LPcap::openDevice(devName_, snapLen_, promisc_, -1, filter_);
}

bool LPcapDevice::doClose() {
	return LPcap::doClose();
}

LPacket::Result LPcapDevice::read(LPacket* packet) {
	LPacket::Result	res = LPcap::read(packet);
	if (res == LPacket::Ok)
		packet->len_ += adjustFrameSize_;
	if (res == LPacket::Timeout)
		std::this_thread::sleep_for(std::chrono::milliseconds(waitTimeout_));
	return res;
}

#ifdef GTEST
#include <gtest/gtest.h>

TEST(PcapDevice, openCloseTest) {
	LPcapDevice device;
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
