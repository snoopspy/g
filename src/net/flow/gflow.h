// ----------------------------------------------------------------------------
//
// G Library
//
// http://www.gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include <QVector>

// ----------------------------------------------------------------------------
// GFlow
// ----------------------------------------------------------------------------
namespace GFlow {
  // --------------------------------------------------------------------------
  // Value
  // --------------------------------------------------------------------------
  struct Value {
    // size_t         packets; // gilgil temp 2016.10.10
    // size_t         bytes; // gilgil temp 2016.10.10
    struct timeval ts_;
    // bool           created; // gilgil temp 2016.10.10
    u_char* totalMem_;
    u_char* mem(size_t offset) { return totalMem_ + offset; }
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

    size_t requestMemory(void* id, size_t memSize) {
      size_t currentOffset = 0;
      foreach (const RequestItem& item, *this) {
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
}

