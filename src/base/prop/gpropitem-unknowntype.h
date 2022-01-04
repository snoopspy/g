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

#include "gpropitem.h"

// ----------------------------------------------------------------------------
// GPropItemUnknownType
// ----------------------------------------------------------------------------
struct G_EXPORT GPropItemUnknownType : public GPropItem {\
	Q_OBJECT

public:
	GPropItemUnknownType(GPropItemParam* param);
	void update() override {}
};

#endif // QT_GUI_LIB
