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

#include <QWidget>
#include <QHBoxLayout>
#include <QToolButton>

#include "base/prop/gprop.h"

// ----------------------------------------------------------------------------
// GDefaultWidget
// ----------------------------------------------------------------------------
struct G_EXPORT GDefaultWidget : QWidget {
	Q_OBJECT

public:
	GDefaultWidget(QWidget* parent = nullptr);
	~GDefaultWidget() override;

public:
	QVBoxLayout* mainLayout_{nullptr};
	QHBoxLayout* toolButtonLayout_{nullptr};

	QToolButton* tbStart_{nullptr};
	QToolButton* tbStop_{nullptr};
	QToolButton* tbOption_{nullptr};

private slots:
	void processSignal(int signo);
};

#endif
