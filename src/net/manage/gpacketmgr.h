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

#include "base/gstateobj.h"
#include "net/packet/gpacket.h"
#include "gflowkey.h"

// ----------------------------------------------------------------------------
// GPacketMgr
// ----------------------------------------------------------------------------
struct G_EXPORT GPacketMgr : GStateObj {
	Q_OBJECT
	Q_PROPERTY(long checkIntervalSec MEMBER checkIntervalSec_)

public:
	long checkIntervalSec_{1}; // 1 second

public:
	GPacketMgr(QObject* parent = nullptr) : GStateObj(parent) {}
	~GPacketMgr() override { close(); }

protected:
	bool doOpen() override {
		lastCheckClock_ = 0;
		return true;
	}

	bool doClose() override {
		return true;
	}

protected:
	long lastCheckClock_{0};

public:
	// --------------------------------------------------------------------------
	// Value
	// --------------------------------------------------------------------------
	struct Value {
		struct timeval firstTs_;
		struct timeval lastTs_;

		static struct Value* allocate(size_t totalMemSize) {
			Value* res = reinterpret_cast<Value*>(malloc(sizeof(Value) + totalMemSize));
			new (res) Value;
			return res;
		}

		static void deallocate(Value* value) {
			value->~Value();
			free(static_cast<void*>(value));
		}

		void* mem(size_t offset) { return pbyte(this) + sizeof(Value) + offset; }
	};

	// ----------------------------------------------------------------------------
	// RequestItem
	// ----------------------------------------------------------------------------
	struct RequestItem {
		void* id_{nullptr};
		size_t offset_{0};
		size_t memSize_{0};
	};

	// ----------------------------------------------------------------------------
	// RequestItems
	// ----------------------------------------------------------------------------
	struct RequestItems : QVector<RequestItem> {
		size_t totalMemSize_{0};

		size_t request(void* id, size_t memSize) {
			size_t currentOffset = 0;
			for (RequestItem& item: *this) {
				if (item.id_ == id) return currentOffset;
				currentOffset += item.memSize_;
			}

			RequestItem newItem;
			newItem.id_ = id;
			newItem.offset_ = currentOffset;
			newItem.memSize_ = memSize;

			push_back(newItem);
			totalMemSize_ += memSize;

			return currentOffset;
		}
	};
};
