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

#include <errno.h>
#include <QDebug>
#include <QSharedPointer>
#include <QString>
#include "gerrcategory.h"
#include "gbase.h"

// ----------------------------------------------------------------------------
// GErr
// ----------------------------------------------------------------------------
struct G_EXPORT GErr {
	enum {
		Ok = 0,
		Fail = -1,
		Unknown = GErrCategory::Base,
		NotSupported,
		NotOpenedState,
		NotClosedState,
		VirtualFunctionCall,
		UnreachableCode,
		ValueIsNull,
		ValueIsNotNull,
		ValueIsZero,
		ValueIsNotZero,
		ObjectIsNull,
		ObjectIsNotNull,
		ReturnNull,
		ReturnNotNull,
		FileNameNotSpecified,
		InterfaceNameNotSpecified,
		ConnectFailed,
		DisconnectFailed,
		ReadFailed,
		WriteFailed
	};

	GErr() : code_(Ok) {}
	GErr(int code) : code_(code) {}
	GErr(int code, QString msg) : code_(code), msg_(msg) {}
	virtual ~GErr() {}

	virtual const char* name() { return "Err"; }
	virtual int code() { return code_; }
	virtual QString msg() { return msg_; }

protected:
	int code_;
	QString msg_;
};

QDebug G_EXPORT operator << (QDebug os, GErr& err);
QDebug G_EXPORT operator << (QDebug os, GErr&& err);
QDebug G_EXPORT operator << (QDebug os, GErr* err);
QDebug G_EXPORT operator << (QDebug os, QSharedPointer<GErr> err);

// ----------------------------------------------------------------------------
// GLastErr
// ----------------------------------------------------------------------------
struct G_EXPORT GLastErr : public GErr {
	GLastErr() {
		code_ = errno;
		msg_ = strerror(code_);
	}

	const char* name() override { return "LastErr"; }
};
