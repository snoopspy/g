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

#ifdef QT_GUI_LIB

#include "glog.h"

// ----------------------------------------------------------------------------
// GLogQObject
// ----------------------------------------------------------------------------
struct G_EXPORT GLogQObject : GLog {
	Q_OBJECT

public:
	Q_INVOKABLE GLogQObject(QObject* parent = nullptr) : GLog(parent) {}
	~GLogQObject() override {}

	void write(QString& msg) override;

signals:
	void writeLogRequred(QString msg);
};

#endif // QT_GUI_LIB
