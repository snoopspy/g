#include "gthreaddelay.h"
#include "net/packet/gethpacket.h"
#include "net/packet/gippacket.h"
#include "net/packet/gdot11packet.h"
#include "net/packet/gnullpacket.h"

// ----------------------------------------------------------------------------
// GThreadDelay
// ----------------------------------------------------------------------------
bool GThreadDelay::doOpen() {
	rg_.seed();
	bufferThread_.start();
	return true;
}

bool GThreadDelay::doClose() {
	bufferThread_.swe_.wakeAll();
	bufferThread_.quit();
	bufferThread_.wait();
	return true;
}

void GThreadDelay::BufferThread::run() {
	while (threadDelay_->active()) {
		lock();
		ssize_t mapSize = map_.size();
		if (mapSize == 0) {
			unlock();
			qDebug() << "mapSize is zero";
			swe_.wait();
		} else {
			GDuration timeout = ULONG_MAX;
			qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
			for (BufferMap::iterator it = map_.begin(); it != map_.end();) {
				qint64 ts = it.key();
				if (ts > now) {
					timeout = ts - now;
					break;
				}

				GPacket* packet = it.value();
				qDebug() << "emit"; // gilgil temp 2024.10.07
				emit threadDelay_->delayed(packet);
				free(packet->buf_.data_);
				delete packet;
				it = map_.erase(it);
			}
			unlock();
			qDebug() << "timeout =" << timeout;
			swe_.wait(timeout);
		}
	}
}

void GThreadDelay::delay(GPacket* packet) {
	qDebug() << ""; // gilgil temp 2024.10.07
	GPacket::Dlt dlt = packet->dlt();
	GPacket* newPacket;
	switch (dlt) {
		case GPacket::Eth: newPacket = new GEthPacket; break;
		case GPacket::Ip: newPacket = new GIpPacket; break;
		case GPacket::Dot11: newPacket = new GDot11Packet; break;
		case GPacket::Null: newPacket = new GNullPacket; break;
	}
	size_t size = packet->buf_.size_;
	gbyte* data = pbyte(malloc(packet->buf_.size_));
	memcpy(data, packet->buf_.data_, size);
	GBuf buf(data, size);
	newPacket->copyFrom(packet, buf);

	packet->ctrl_.block_ = true;

	qint64 ts = packet->ts_.tv_sec;
	ts = ts * 1000 + packet->ts_.tv_usec / 1000;
	ts += timeout_ + jitter_ * (rg_.generate() % 1024) / 1024;

	bool wake = false;
	{
		QMutexLocker ml(&bufferThread_);
		BufferThread::BufferMap* map = &bufferThread_.map_;
		if (map->size() == 0) wake = true;
		else {
			qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
			BufferThread::BufferMap::iterator it = map->begin();
			qint64 ts = it.key();
			if (ts <= now)
				wake = true;
		}
		bufferThread_.map_.insert(ts, newPacket);
	}
	qDebug() << "wake=" << wake;
	if (wake)
		bufferThread_.swe_.wakeAll();
}
