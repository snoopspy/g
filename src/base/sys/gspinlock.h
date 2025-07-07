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

#include <atomic>

// ----------------------------------------------------------------------------
// GSpinLock
// ----------------------------------------------------------------------------
struct GSpinLock {
	std::atomic_flag locked = ATOMIC_FLAG_INIT;

public:
	void lock() {
		while (locked.test_and_set(std::memory_order_acquire)) {}
	}
	void unlock() {
		locked.clear(std::memory_order_release);
	}
};

// ----------------------------------------------------------------------------
// GSpinLockGuard
// ----------------------------------------------------------------------------
struct GSpinLockGuard {
protected:
	GSpinLock* spinLock_;

public:
	GSpinLockGuard(GSpinLock& spinLock) : spinLock_(&spinLock) { spinLock_->lock(); }
	~GSpinLockGuard() { spinLock_->unlock(); }
};
