#include "gtimestampsyncdelay.h"
#include <QDeadlineTimer>
#include <QThread>

// ----------------------------------------------------------------------------
// GTimeStampSyncDelay
// ----------------------------------------------------------------------------
bool GTimeStampSyncDelay::doOpen() {
	et_.restart();
	lastClock_ = et_.elapsed();
	lastTs_ = 0;
	return true;
}

bool GTimeStampSyncDelay::doClose() {
	swe_.wakeAll();
	return true;
}

bool GTimeStampSyncDelay::delay(GPacket* packet) {
	qint64 nowTs = qint64(packet->ts_.tv_sec * 1000) + qint64(packet->ts_.tv_usec / 1000);
	if (lastTs_ == 0) lastTs_ = nowTs;
	qint64 remainTs = nowTs - lastTs_;
	// qDebug() << "remainTs=" << remainTs << "nowTs=" << nowTs << "lastTs_=" << lastTs_; // gilgil temp 2019.05.20
	lastTs_ = nowTs;
	if (speed_ != 1.)
		remainTs *= speed_;

	qint64 nowClock = et_.elapsed();
	qint64 remain = nowClock - lastClock_;
	// qDebug() << "remain=" << remain << "now=" << nowClock << "last_=" << lastClock_; // gilgil temp 2019.05.20

	remainTs -= remain;

	if (remainTs < 0) {
		qCritical() << "remain is " << remain;
		remainTs = 0;
	}
	//qDebug() << "remainTs=" << remainTs; // gilgil temp 2019.05.21

	QDeadlineTimer dt(remainTs);
	bool res = swe_.wait(dt);
	lastClock_ = et_.elapsed();
	if (res == false) // timeout elapsed
		emit delayed(packet);
	return res;
}
