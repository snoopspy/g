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
#include "net/write/gwrite.h"
#include "net/packet/gethpacket.h"
#include "net/packet/gippacket.h"

// ----------------------------------------------------------------------------
// GTcpBlock
// ----------------------------------------------------------------------------
struct G_EXPORT GTcpBlock : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(BlockType forwardBlockType MEMBER forwardBlockType_)
	Q_PROPERTY(QStringList forwardFinMsg MEMBER forwardFinMsg_)
	Q_PROPERTY(BlockType backwardBlockType MEMBER backwardBlockType_)
	Q_PROPERTY(QStringList backwardFinMsg MEMBER backwardFinMsg_)
	Q_PROPERTY(int bufSize MEMBER bufSize_)
	Q_PROPERTY(GObjPtr writer READ getWriter WRITE setWriter)
	Q_ENUMS(BlockType)

public:
	enum BlockType {
		None,
		Rst,
		Fin
	};
public:
	GObjPtr getWriter() { return writer_; }
	void setWriter(GObjPtr value) { writer_ = dynamic_cast<GWrite*>(value.data()); }

public:
	bool enabled_{true};
	BlockType forwardBlockType_{Rst};
	QStringList forwardFinMsg_;
	BlockType backwardBlockType_{Rst};
	QStringList backwardFinMsg_;
	int bufSize_{GPacket::MaxBufSize};
	GWrite* writer_{nullptr};

public:
	Q_INVOKABLE GTcpBlock(QObject* parent = nullptr) : GStateObj(parent) {}
	~GTcpBlock() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	QString forwardFinMsgStr_;
	QString backwardFinMsgStr_;

	GEthPacket blockEthPacket_;
	GIpPacket blockIpPacket_;
	gbyte* blockBuf_{nullptr};

	enum Direction {
		Forward,
		Backward
	};

	void sendBlockPacket(GPacket* packet, Direction direction, BlockType blockType, uint32_t seq, uint32_t ack, QString msg = "");

protected:
	GMac myMac_;

public slots:
	void block(GPacket* packet);

signals:
	void blocked(GPacket* packet);
};
