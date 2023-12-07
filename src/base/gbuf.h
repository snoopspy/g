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

#include "gbase.h"

// ----------------------------------------------------------------------------
// GBuf
// ----------------------------------------------------------------------------
struct QString;
struct G_EXPORT GBuf final {
	gbyte* data_;
	size_t size_;

	GBuf() {}
	GBuf(unsigned char* data, size_t size) : data_(data), size_(size) {}

	void clear() {
		data_ = nullptr;
		size_ = 0;
	}

	bool valid() {
		return data_ != nullptr;
	}

	bool contains(void* p) {
		if (p < data_) return false;
		if (p >= data_ + size_) return false;
		return true;
	}

	bool contains(GBuf o) {
		if (o.data_ < data_) return false;
		if (o.data_ + o.size_ > data_ + size_) return false;
		return true;
	}

	explicit operator QString() const;
};
