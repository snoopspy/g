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

#include <GCheckBox>
#include "gpropitem.h"

// ----------------------------------------------------------------------------
// GPropItemCheckBox
// ----------------------------------------------------------------------------
struct G_EXPORT GPropItemCheckBox : public GPropItem {
	Q_OBJECT

public:
	GPropItemCheckBox(GPropItemParam* param);

public:
	GCheckBox* checkBox_{nullptr};
};

#endif // QT_GUI_LIB
