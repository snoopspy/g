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
#include "net/gnetinfo.h"
#include "net/packet/gpacket.h"

// ----------------------------------------------------------------------------
// GNeighborBlock
// ----------------------------------------------------------------------------
struct GNeighborBlock : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(QString intfName MEMBER intfName_)

public:
	bool enabled_{true};
	QString intfName_;

public:
	Q_INVOKABLE GNeighborBlock(QObject* parent = nullptr);
	~GNeighborBlock() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GIntf* intf_{nullptr};

public slots:
	void block(GPacket* packet);
	void unblock(GPacket* packet);

signals:
	void blocked(GPacket* packet);
	void unblocked(GPacket* packet);

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
