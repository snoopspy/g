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

// ----------------------------------------------------------------------------
// GTtlReplace
// ----------------------------------------------------------------------------
struct G_EXPORT GTtlReplace : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(bool log MEMBER log_)
	Q_PROPERTY(int ttl MEMBER ttl_)

public:
	bool enabled_{true};
	bool log_{true};
	quint8 ttl_{1};

public:
	Q_INVOKABLE GTtlReplace(QObject* parent = nullptr) : GStateObj(parent) {}
	~GTtlReplace() override {}

protected:
	bool doOpen() override { return true; }
	bool doClose() override { return true; }

public slots:
	void replace(GPacket* packet);

signals:
	void replaced(GPacket* packet);
	void notReplaced(GPacket* packet);
};
