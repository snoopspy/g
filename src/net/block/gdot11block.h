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
#include "net/write/gwrite.h"

// ----------------------------------------------------------------------------
// GDot11Block
// ----------------------------------------------------------------------------
struct G_EXPORT GDot11Block : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(bool debugLog MEMBER debugLog_)
	Q_PROPERTY(bool authStaAp MEMBER authStaAp_);
	Q_PROPERTY(bool deauthApBroadcast MEMBER deauthApBroadcast_);
	Q_PROPERTY(bool deauthStaAp MEMBER deauthStaAp_);
	Q_PROPERTY(bool disassociateStaAp MEMBER disassociateStaAp_);
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(GObjPtr writer READ getWriter WRITE setWriter)

public:
	GObjPtr getWriter() { return writer_; }
	void setWriter(GObjPtr value) { writer_ = dynamic_cast<GWrite*>(value.data()); }

public:
	bool enabled_{true};
	bool debugLog_{false};
	bool authStaAp_{false};
	bool deauthApBroadcast_{true};
	bool deauthStaAp_{false};
	bool disassociateStaAp_{false};
	GDuration sendInterval_{100};
	GWrite* writer_{nullptr};

public:
	Q_INVOKABLE GDot11Block(QObject* parent = nullptr);
	~GDot11Block() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	QAtomicInt currentChannel_{0};


public slots:
	void block(GPacket* packet);
	void processChannelChanged(int channel);

signals:
	void blocked(GPacket* packet);
};
