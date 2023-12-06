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

#include "gpcapcapture.h"

// ----------------------------------------------------------------------------
// GPcapFile
// ----------------------------------------------------------------------------
struct G_EXPORT GPcapFile : GPcapCapture {
	Q_OBJECT
	Q_PROPERTY(QString fileName MEMBER fileName_)
	Q_PROPERTY(int loopCount MEMBER loopCount_)
	Q_PROPERTY(QStringList frameNumbers MEMBER frameNumbers_)

public:
	QString fileName_{""};
	int loopCount_{1};
	QStringList frameNumbers_;

public:
	Q_INVOKABLE GPcapFile(QObject* parent = nullptr) : GPcapCapture(parent) {}
	~GPcapFile() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPacket::Result read(GPacket* packet) override;

protected:
	int loopIndex_;
	int currentFrameNumber_;
	QSet<int>frameNumberSet_;

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
