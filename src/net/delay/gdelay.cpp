#include "gdelay.h"

// ----------------------------------------------------------------------------
// GDelay
// ----------------------------------------------------------------------------
bool GDelay::doOpen() {
	return true;
}

bool GDelay::doClose() {
	swe_.wakeAll();
	return true;
}

bool GDelay::delay(GPacket* packet) {
	bool res = swe_.wait(timeout_);
	if (res == false) // timeout elapsed
		emit delayed(packet);
	return res;
}
