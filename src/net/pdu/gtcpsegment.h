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

#include <QMap>
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
	struct Map : QMap<uint32_t /*seq*/, QByteArray /*segment*/> {
	} map_;
	uint32_t firstSeq_;
	uint32_t nextSeq_;
	QByteArray reassemble();
};
