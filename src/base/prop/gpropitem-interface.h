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

#include "gpropitem-combobox.h"

// ----------------------------------------------------------------------------
// GPropItemInterface
// ----------------------------------------------------------------------------
struct G_EXPORT GPropItemInterface : public GPropItemComboBox {
	Q_OBJECT

public:
	GPropItemInterface(GPropItemParam* param);
	void update() override;

protected:
	QStringList intfNames_;

protected slots:
	void myCurrentIndexChanged(int index);
};

#endif // QT_GUI_LIB
