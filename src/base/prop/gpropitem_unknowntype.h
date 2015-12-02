// ----------------------------------------------------------------------------
//
// G Library
//
// http://www.gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#ifdef QT_GUI_LIB

#include <QCheckBox>
#include "gpropitem.h"

// ----------------------------------------------------------------------------
// GPropItemUnknownType
// ----------------------------------------------------------------------------
struct GPropItemUnknownType : public GPropItem {
  // Q_OBJECT // gilgil temp 2015.12.02

public:
  GPropItemUnknownType(QTreeWidgetItem* parent, QObject* object, QMetaProperty mpro);
  void update() override {}
};

#endif // QT_GUI_LIB