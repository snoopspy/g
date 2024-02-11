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

#include <QList>

// ----------------------------------------------------------------------------
// GIw
// ----------------------------------------------------------------------------
struct GIw {
public:
	GIw();
	GIw(QString intfName);
	virtual ~GIw();

protected:
	QString intfName_;

public:
	static const int BufSize = 256;
	static const int InvalidChannel = 0;
	QString error_;

public:
	bool open(QString intfName);
	bool close();

	int channel();
	bool setChannel(int channel);
	QList<int> channelList() { return channelList_; }

protected:
	QList<int> channelList_;

public:
	static int ieee80211_frequency_to_channel(int freq);
};
