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
#include "net/process/gcorrectchecksum.h"

// ----------------------------------------------------------------------------
// GTtlReplace
// ----------------------------------------------------------------------------
struct G_EXPORT GTtlReplace : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(bool log MEMBER log_)
	Q_PROPERTY(int ttl MEMBER ttl_)
	Q_PROPERTY(GObjRef correctChecksum READ getCorrectChecksum)

	GObjRef getCorrectChecksum() { return &correctChecksum_; }

public:
	bool enabled_{true};
	bool log_{true};
	quint8 ttl_{1};
	GCorrectChecksum correctChecksum_;

public:
	Q_INVOKABLE GTtlReplace(QObject* parent = nullptr);
	~GTtlReplace() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public slots:
	void replace(GPacket* packet);

signals:
	void replaced(GPacket* packet);
	void notReplaced(GPacket* packet);
};
