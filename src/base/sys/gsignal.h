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

#include <csignal>
#include <QHash>
#include <QObject>

#include "base/gbase.h"

// ----------------------------------------------------------------------------
// GSignal
// ----------------------------------------------------------------------------
struct G_EXPORT GSignal : QObject {
	Q_OBJECT
	Q_PROPERTY(bool signalOnce MEMBER signalOnce_)
	Q_PROPERTY(bool callOriginFunc MEMBER callOriginFunc_)

private: // singleton
	GSignal();
	~GSignal() override;

public:
	bool signalOnce_{true};
	bool callOriginFunc_{false};

public:
#ifdef Q_OS_LINUX
	typedef __sighandler_t Handler;
#endif
#ifdef Q_OS_WIN
	typedef __p_sig_fn_t Handler;
#endif
protected:
	typedef QHash<int, Handler> Handlers;
	Handlers handlers_;
	void signalHandler(int signo);
	static void _signalHandler(int signo);

public:
	void setup(int signo, Handler handler = nullptr);
	void setupAll(Handler handler = nullptr);
	void ignore(int signo);
	static QString getString(int signo);

	static GSignal& instance() {
		static GSignal signal;
		return signal;
	}

signals:
	void signaled(int signo);
};
