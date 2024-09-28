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

#include <map>
#include <set>
#include <QByteArray>
#include <QRecursiveMutex>

// ----------------------------------------------------------------------------
// GTcpSegment
// ----------------------------------------------------------------------------
struct GTcpSegment : QByteArray {
public:
	GTcpSegment(uint32_t firstSeq);
	QByteArray insert(uint32_t seq, QByteArray segment);
	QRecursiveMutex mutex_;

protected:
	struct Map : std::map<uint32_t /*seq*/, QByteArray /*segment*/> {} map_;
	struct Set : std::set<int32_t /*seq*/> {} set_;
	uint32_t firstSeq_;
	uint32_t nextSeq_;
	QByteArray reassemble();
};
