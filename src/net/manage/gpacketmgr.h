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
#ifdef _DEBUG
	uint32_t canary_{0x43434343}; // CCCC
#endif

	enum Direction {
		ClientToServer,
		ServerToClient
	};

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
	GPacket* currentPacket_{nullptr};

public:
	// --------------------------------------------------------------------------
	// Value
	// --------------------------------------------------------------------------
	struct Value {
		struct timeval firstTime_{0, 0};
		struct timeval lastTime_{0 ,0};
#ifdef _DEBUG
		size_t totalMemSize_{0};
#endif // _DEBUG

		static struct Value* allocate(size_t totalMemSize) {
			Value* value = reinterpret_cast<Value*>(malloc(sizeof(Value) + totalMemSize));
			new (value) Value;
#ifdef _DEBUG
			value->totalMemSize_ = totalMemSize;
			memset(pbyte(value) + sizeof(Value), 'A', totalMemSize);
#endif // _DEBUG
			return value;
		}

		static void deallocate(Value* value) {
#ifdef _DEBUG
			memset(pbyte(value) + sizeof(Value), 'B', value->totalMemSize_);
#endif // _DEBUG
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
