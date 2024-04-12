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
#include "net/write/grawipsocketwrite.h"
#include "net/packet/gippacket.h"

// ----------------------------------------------------------------------------
// GUdpBlock
// ----------------------------------------------------------------------------
struct G_EXPORT GUdpBlock : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(BlockType blockType MEMBER blockType_)
	Q_PROPERTY(int maxBlockSize MEMBER maxBlockSize_)
	Q_PROPERTY(GObjRef writer READ getWriter)

	Q_ENUMS(BlockType)

public:
	enum BlockType {
		L2,
		L3,
		Payload
	};
public:
	GObjRef getWriter() { return &writer_; }

public:
	bool enabled_{true};
	BlockType blockType_{Payload};
	int maxBlockSize_{GPacket::MtuSize};
	GRawIpSocketWrite writer_{this};

public:
	Q_INVOKABLE GUdpBlock(QObject* parent = nullptr) : GStateObj(parent) {}
	~GUdpBlock() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GIpPacket blockIpPacket_;
	QByteArray blockByteArray_;

public slots:
	void block(GPacket* packet);

signals:
	void blocked(GPacket* packet);
};
