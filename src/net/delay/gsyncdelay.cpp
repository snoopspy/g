#include "gsyncdelay.h"
#include <QDeadlineTimer>

// ----------------------------------------------------------------------------
// GSyncDelay
// ----------------------------------------------------------------------------
bool GSyncDelay::doOpen() {
	et_.restart();
	lastClock_ = et_.elapsed();
	return true;
}

bool GSyncDelay::doClose() {
	swe_.wakeAll();
	return true;
}

bool GSyncDelay::delay(GPacket* packet) {
	qint64 nowClock = et_.elapsed();
	qint64 remain = qint64(timeout_) - (nowClock - lastClock_);
	qDebug() << "remain=" << remain << "nowClock=" << nowClock << "lastClock_=" << lastClock_; // gilgil temp 2019.05.19
	if (speed_ != 1.)
		remain *= speed_;

	if (remain < 0) {
		qCritical() << "remain is " << remain;
		remain = 0;
	}

	QDeadlineTimer dt(remain);
	bool res = swe_.wait(dt);
	lastClock_ = et_.elapsed();
	if (res == false) // timeout elapsed
		emit delayed(packet);
	return res;
}
