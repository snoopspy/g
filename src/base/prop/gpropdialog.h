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

#include <QDialog>
#include "gpropwidget.h"

// ----------------------------------------------------------------------------
// GPropDialog
// ----------------------------------------------------------------------------
struct G_EXPORT GPropDialog : public QDialog, GProp {
	GPropDialog(QWidget *parent = nullptr);
	~GPropDialog() override;

public:
	GPropWidget widget_;

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};

#endif
