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
#include "net/write/gvirtualpcapdevicedrite.h"
#include "net/packet/gethpacket.h"
#include "net/packet/gippacket.h"

// ----------------------------------------------------------------------------
// GTcpBlock
// ----------------------------------------------------------------------------
struct G_EXPORT GTcpBlock : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(bool forwardRst MEMBER forwardRst_)
	Q_PROPERTY(bool forwardFin MEMBER forwardFin_)
	Q_PROPERTY(QStringList forwardFinMsg MEMBER forwardFinMsg_)
	Q_PROPERTY(bool backwardRst MEMBER backwardRst_)
	Q_PROPERTY(bool backwardFin MEMBER backwardFin_)
	Q_PROPERTY(QStringList backwardFinMsg MEMBER backwardFinMsg_)
	Q_PROPERTY(int bufSize MEMBER bufSize_)
	Q_PROPERTY(GObjPtr writer READ getWriter WRITE setWriter)

public:
	GObjPtr getWriter() { return writer_; }
	void setWriter(GObjPtr value) { writer_ = dynamic_cast<GVirtualPcapDeviceWrite*>(value.data()); }

public:
	GVirtualPcapDeviceWrite* writer_{nullptr};

public:
	bool enabled_{true};
	bool forwardRst_{true};
	bool forwardFin_{false};
	QStringList forwardFinMsg_;
	bool backwardRst_{true};
	bool backwardFin_{false};
	int bufSize_{GPacket::MaxBufSize};
	QStringList backwardFinMsg_;

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

	enum BlockType {
		Rst,
		Fin
	};
	void sendBlockPacket(GPacket* packet, Direction direction, BlockType blockType, uint32_t seq, uint32_t ack, QString msg = "");

protected:
	GMac myMac_;

public slots:
	void block(GPacket* packet);

signals:
	void blocked(GPacket* packet);
};
