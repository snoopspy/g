// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include <climits>
#include <QDeadlineTimer>
#include <QWaitCondition>
#include <QMutex>

#include "base/gbase.h"

// ----------------------------------------------------------------------------
// GWaitEvent
// ----------------------------------------------------------------------------
struct G_EXPORT GWaitEvent {
	QMutex m_;
	QWaitCondition wc_;

	bool wait(GDuration timeout = ULONG_MAX) {
		m_.lock();
		bool res = wc_.wait(&m_, timeout);
		m_.unlock();
		return res;
	}

	bool wait(QDeadlineTimer& dt) {
		m_.lock();
		bool res = wc_.wait(&m_, dt);
		m_.unlock();
		return res;
	}

	void wakeOne() {
		m_.lock();
		wc_.wakeOne();
		m_.unlock();
	}

	void wakeAll() {
		m_.lock();
		wc_.wakeAll();
		m_.unlock();
	}
};

// ----------------------------------------------------------------------------
// GStateWaitEvent
// ----------------------------------------------------------------------------
struct G_EXPORT GStateWaitEvent : GWaitEvent {
	bool state_;

	GStateWaitEvent(bool state = false) : state_(state) {}

	bool wait(GDuration timeout = ULONG_MAX) {
		m_.lock();
		if (state_) {
			state_ = false;
			m_.unlock();
			return true;
		}
		bool res = wc_.wait(&m_, timeout);
		if (res)
			state_ = false;
		m_.unlock();
		return res;
	}

	bool wait(QDeadlineTimer& dt) {
		m_.lock();
		if (state_) {
			state_ = false;
			m_.unlock();
			return true;
		}
		bool res = wc_.wait(&m_, dt);
		if (res)
			state_ = false;
		m_.unlock();
		return res;
	}

	void wakeOne() {
		m_.lock();
		state_ = true;
		wc_.wakeOne();
		m_.unlock();
	}

	void wakeAll() {
		m_.lock();
		state_ = true;
		wc_.wakeAll();
		m_.unlock();
	}
};

// ----------------------------------------------------------------------------
// GCountWaitEvent
// ----------------------------------------------------------------------------
struct G_EXPORT GCountWaitEvent : GWaitEvent {
	int count_;

	GCountWaitEvent(int count = 0) : count_(count) {}

	bool wait(GDuration timeout = ULONG_MAX) {
		m_.lock();
		if (count_ > 0) {
			count_--;
			m_.unlock();
			return true;
		}
		bool res = wc_.wait(&m_, timeout);
		if (res)
			count_--;
		m_.unlock();
		return res;
	}

	bool wait(QDeadlineTimer& dt) {
		m_.lock();
		if (count_ > 0) {
			count_--;
			m_.unlock();
			return true;
		}
		bool res = wc_.wait(&m_, dt);
		if (res)
			count_--;
		m_.unlock();
		return res;
	}

	void wakeOne() {
		m_.lock();
		count_++;
		wc_.wakeOne();
		m_.unlock();
	}

	void wakeAll() {
		m_.lock();
		count_++;
		wc_.wakeAll();
		m_.unlock();
	}
};
