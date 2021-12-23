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

#include <QMutex>
#include <QWaitCondition>
#include "gstateobj.h"

// ----------------------------------------------------------------------------
// GRwPool
// ----------------------------------------------------------------------------
template <typename T>
struct G_EXPORT GRwPool : GStateObj {
	GRwPool(QObject* parent = nullptr) : GStateObj(parent) {
	}

	~GRwPool() {
		close();
		if (items_ != nullptr) {
			delete[] items_;
			items_ = nullptr;
		}
	}

	void setSize(int size) {
		size_ = size;
	}

	T read() {
		if (!active())
			return T(0);

		mutex_.lock();
		if (index_ == 0)
			notEmpty_.wait(&mutex_);
		mutex_.unlock();

		if (!active())
			return T(0);

		mutex_.lock();
		Q_ASSERT(index_ > 0);
		T res = items_[--index_];
		notFull_.wakeAll();
		mutex_.unlock();

		return res;
	}

	void write(T t) {
		if (!active())
			return;

		mutex_.lock();
		if (index_ + 1 >= size_)
			notFull_.wait(&mutex_);
		mutex_.unlock();

		if (!active())
			return;

		mutex_.lock();
		Q_ASSERT(index_ < size_);
		items_[index_++] = t;
		notEmpty_.wakeAll();
		mutex_.unlock();
	}

protected:
	bool doOpen() override {
		if (size_ == 0) {
			SET_ERR(GErr::ValueIsZero, "count_ is zero");
			return false;
		}
		if (items_ != nullptr)
			delete[] items_;
		items_ = new T[size_];
		return true;
	}

	bool doClose() override {
		notEmpty_.wakeAll();
		notFull_.wakeAll();
		return true;
	}

protected:
	size_t size_{0};
	size_t index_{0};
	T* items_{nullptr};

	QWaitCondition notEmpty_;
	QWaitCondition notFull_;
	QMutex mutex_;
};
