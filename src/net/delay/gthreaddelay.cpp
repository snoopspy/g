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
	bufferThread_.priority_ = QThread::TimeCriticalPriority;
	bufferThread_.start();
	return true;
}

bool GThreadDelay::doClose() {
	if (flushOnClose_) {
		QMutexLocker ml(&bufferThread_);
		BufferThread::BufferMap* map = &bufferThread_.map_;
		for (BufferThread::BufferMap::iterator it = map->begin(); it != map->end();) {
			GPacket* packet = it.value();
			emit delayed(packet);
			free(packet->buf_.data_);
			it = map->erase(it);
		}
	}
	bufferThread_.swe_.wakeAll();
	bufferThread_.quit();
	bufferThread_.wait();
	return true;
}

void GThreadDelay::BufferThread::run() {
	qDebug() << priority();
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
			int emitCount = 0;
			for (BufferMap::iterator it = map_.begin(); it != map_.end();) {
				qint64 ts = it.key();
				if (now < ts) {
					timeout = ts - now;
					break;
				}

				GPacket* packet = it.value();
				emit threadDelay_->delayed(packet);
				emitCount++;
				free(packet->buf_.data_);
				delete packet;
				it = map_.erase(it);
			}
			unlock();
			qDebug() << "emit =" << emitCount << " timeout =" << timeout;
			swe_.wait(timeout);
		}
	}
}

static qint64 lastTs = 0; // gilgil temp 2024.10.08
void GThreadDelay::delay(GPacket* packet) {
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
	qDebug() << "ts - lastTs =" << ts - lastTs; lastTs = ts; // gilgil temp 2024.10.08

	bool wake = false;
	{
		QMutexLocker ml(&bufferThread_);
		BufferThread::BufferMap* map = &bufferThread_.map_;
		if (map->size() == 0) wake = true;
		else {
			BufferThread::BufferMap::iterator it = map->begin();
			qint64 firstTs = it.key();
			qDebug() << "diff =" << ts - firstTs; // gilgil temp 2024.10.08
			if (ts < firstTs) {
				qDebug() << "first wake!!!";
				wake = true;
			}
		}
		bufferThread_.map_.insert(ts, newPacket);
	}
	if (wake)
		bufferThread_.swe_.wakeAll();
}
