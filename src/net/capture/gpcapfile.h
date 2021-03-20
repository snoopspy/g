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

#include "gpcap.h"

// ----------------------------------------------------------------------------
// GPcapFile
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapFile : GPcap {
	Q_OBJECT
	Q_PROPERTY(QString fileName MEMBER fileName_)

public:
	QString fileName_{""};

public:
	Q_INVOKABLE GPcapFile(QObject* parent = nullptr) : GPcap(parent) {}
	~GPcapFile() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
