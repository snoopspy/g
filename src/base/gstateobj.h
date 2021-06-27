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

#include "gobj.h"

// ----------------------------------------------------------------------------
// GStateObj
// ----------------------------------------------------------------------------
struct G_EXPORT GStateObj : GObj {
	Q_OBJECT

public:
	typedef enum {
		Closed, // 0
		Opening, // 1
		Opened, // 2
		Closing // 3
	} State;

	GStateObj(QObject* parent = nullptr) : GObj(parent) {}
	~GStateObj() override;

	bool active();

public slots:
	virtual bool open();
	virtual bool close();

signals:
	void opened();
	void closed();

protected:
	virtual bool doOpen();
	virtual bool doClose();

protected:
	State state_{Closed};
};
